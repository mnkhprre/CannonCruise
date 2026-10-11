#!/usr/bin/env python3
"""Clang-only source precheck, NOT a CMake/MSVC build or SDK ABI check.

Runs the non-Windows CMake source set plus Ocean_W32.cpp as a separate probe.
Also replays selected startup declarations/calls against rwcore.h in isolation;
this does not pretend to compile Windows sources or provide Windows headers.
Writes a fresh JSON report and returns nonzero if any probe fails.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
CODE = ROOT / 'code'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='clang++')
    parser.add_argument('--output', required=True, help='New JSON path inside this repository')
    args = parser.parse_args()
    output = (ROOT / args.output).resolve()
    if ROOT not in output.parents:
        parser.error('Output must be inside this repository')
    if output.exists():
        parser.error('Report already exists; choose a new filename to preserve evidence')
    version = subprocess.run([args.compiler, '--version'], capture_output=True, text=True)
    if version.returncode or 'clang' not in version.stdout.lower():
        parser.error('This precheck requires a working Clang driver; use BUILDING.md for MSVC')
    base = [args.compiler, '-std=c++14', '-fsyntax-only', '-ferror-limit=0',
            '-D_POSIX_C_SOURCE=200809L']
    for directory in (CODE, CODE / 'framework', CODE / 'framework/core', CODE / 'framework/toolkits'):
        base += ['-I', str(directory)]
    sources = sorted(p for p in CODE.rglob('*') if p.suffix in ('.cpp', '.c')
                     and 'build' not in p.relative_to(CODE).parts)
    results, skipped = [], []
    for source in sources:
        rel = source.relative_to(CODE).as_posix()
        if rel.startswith('framework/startup/win32/') or rel == 'framework/startup/startup.cpp':
            skipped.append('code/' + rel)
            continue
        group = 'ocean-extra-probe' if rel == 'Ocean_W32.cpp' else 'non-windows-source'
        command = base + [str(source)]
        run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
        results.append(dict(source='code/' + rel, group=group, command=command,
                            returncode=run.returncode, diagnostics=run.stdout + run.stderr))

    startup = (CODE / 'framework/startup/startup.cpp').read_text()
    # Exact statements from the source, not invented replacement SDK signatures.
    names = ('RwEngineTerm', 'RwEngineStop', 'RwEngineClose', 'RwEngineInit')
    declarations = '\n'.join(re.findall(
        r'^extern [^\n]*\b(?:' + '|'.join(names) + r')\([^\n]*;', startup, re.M))
    probes = {
        'startup-declaration-conflicts': '#include "rwcore.h"\n' + declarations + '\n',
        'startup-open-call': '#include "rwcore.h"\nvoid probe() {\n' +
            re.search(r'RwBool openResult = RwEngineOpen\([^\n]*;', startup).group(0) + '\n}\n',
        'startup-video-type': '#include "rwcore.h"\n' +
            re.search(r'^extern RwBool CameraInit\([^\n]*;', startup, re.M).group(0) + '\n',
    }
    for name, text in probes.items():
        command = base + ['-x', 'c++', '-']
        run = subprocess.run(command, input=text, cwd=ROOT, capture_output=True, text=True)
        results.append(dict(source=name, group='isolated-contract-probe', command=command,
                            input=text, returncode=run.returncode, diagnostics=run.stdout + run.stderr))
    hashes = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in sorted(CODE.rglob('*')) if p.suffix in ('.h', '.cpp', '.c', '.txt')
              and p.is_file() and 'build' not in p.relative_to(CODE).parts}
    report = dict(scope='Clang syntax only; no CMake, MSVC, archive, linker or SDK ABI validation',
                  compiler=version.stdout.strip(), source_sha256=hashes,
                  skipped_windows_sources=skipped, results=results)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open('x') as stream:
        json.dump(report, stream, indent=2)
        stream.write('\n')
    for group in ('non-windows-source', 'ocean-extra-probe', 'isolated-contract-probe'):
        selected = [r for r in results if r['group'] == group]
        print(f"{group}: {sum(r['returncode'] == 0 for r in selected)}/{len(selected)} passed")
    print(f'Skipped Windows sources: {len(skipped)}; report: {output.relative_to(ROOT)}')
    return int(any(r['returncode'] != 0 for r in results))


if __name__ == '__main__':
    sys.exit(main())
