#!/usr/bin/env python3
"""FUN_* → Class::Method eşleme tablosu üretici (roadmap 5.5).

code/ altındaki .cpp/.h dosyalarındaki yorumlardan FUN_XXXXXXXX adreslerini
çıkarır, açıklamalarını ve bulunduğu dosyayı eşler; FUN_MAP.md üretir.

Açıklama önceliği:
  1. Aynı satırda `FUN_xxxx — açıklama` kalıbı
  2. Aynı satırda `etiket: \`FUN_xxxx\`` kalıbı (ör. "Dtor: FUN_...")
  3. `FUN_xxxx` geçen satırın dosya/sınıf bağlamı (dosya adından türetilir)

Kullanım:
    python3 tools/extract_fun_map.py
"""

import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CODE = ROOT / "code"
OUT = ROOT / "FUN_MAP.md"

FUN = r"FUN_([0-9a-f]{8})"
# 1) FUN_xxxx — açıklama  /  FUN_xxxx açıklama
AFTER_RE = re.compile(FUN + r"\s*(?:—|-)\s*([^*\n\r`]+)")
# 2) etiket: `FUN_xxxx`  veya  etiket FUN_xxxx
BEFORE_RE = re.compile(r"([A-Za-z][A-Za-z0-9_:~ ]{1,40}?)\s*[: ]\s*`?(" + FUN + r")`?")
SEE_RE = re.compile(r"@see\s+(" + FUN + r")")
MIRROR_RE = re.compile(r"mirrors\s+(" + FUN + r")")
METHOD_RE = re.compile(r"\b([A-Z][A-Za-z0-9_]+)::(~?[A-Za-z0-9_]+)\b")

PUNCT_ONLY = re.compile(r"^[\s`*()\[\]/,.:;|—-]*$")


def clean(text: str) -> str:
    text = re.sub(r"^[\s`*(\[]+", "", text)
    text = re.sub(r"[\s`*)\]:;,.]+$", "", text)  # yalnızca sondaki noktalama
    text = re.sub(r"\s+", " ", text)
    return text.strip()


def file_context(rel: str, text: str) -> str:
    """Dosya bağlamı: dosyadaki ilk class/struct tanımı, yoksa dosya adı."""
    m = re.search(r"\bclass\s+(RWS::)?(C?[A-Z]\w+)", text)
    if m:
        ns = "RWS::" if (m.group(1) or "/framework/" in rel) else ""
        return f"{ns}{m.group(2)} (dosya: {rel})"
    stem = Path(rel).stem
    return f"{stem} (dosya: {rel})"


def main():
    entries = defaultdict(lambda: {"descs": set(), "files": set(), "methods": set()})

    for path in sorted(CODE.rglob("*")):
        if path.suffix not in (".cpp", ".h", ".hpp"):
            continue
        if "build" in path.parts:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        rel = path.relative_to(CODE).as_posix()

        for line in text.splitlines():
            after = {}
            for m in AFTER_RE.finditer(line):
                after[m.group(1)] = clean(m.group(2))

            for m in re.finditer(FUN, line):
                key = m.group(1)
                addr = "FUN_" + key
                e = entries[addr]
                e["files"].add(rel)

                desc = ""
                # Öncelik 1: FUN'dan önceki etiket ("Ctor: FUN_...", "X – mirrors FUN_...")
                prefix = line[: m.start()]
                prefix = re.sub(r"(mirrors|@see|see|CALL|call)\s*$", "", prefix)
                bm = re.search(r"([A-Za-z][A-Za-z0-9_:~]{1,40})\s*[:\-\s]\s*$", prefix)
                label = clean(bm.group(1)) if bm else ""
                STOP = {
                    "ref", "binary", "asm", "addr", "address", "func", "function",
                    "note", "todo", "xref", "ptr", "dat", "lab", "sub", "call",
                    "called", "mirrors", "see", "bin",
                }
                if (
                    label
                    and not label.startswith(("FUN_", "PTR_"))
                    and label.lower() not in STOP
                ):
                    desc = label
                # Öncelik 2: FUN'dan sonra açıklama; baştaki FUN_xx adresi kırpılır
                if not desc:
                    d = after.get(key, "")
                    d = re.sub(r"^(FUN_[0-9a-f]{8}\s*[-—:]\s*)+", "", d).strip()
                    d = re.sub(r"^FUN_[0-9a-f]{8}\s*", "", d).strip()
                    d = clean(d)
                    if d and not PUNCT_ONLY.match(d):
                        desc = d
                if desc and not PUNCT_ONLY.match(desc):
                    e["descs"].add(desc)
                    for mm in METHOD_RE.finditer(desc):
                        e["methods"].add(f"{mm.group(1)}::{mm.group(2)}")

    for path in sorted(CODE.rglob("*")):
        if path.suffix not in (".cpp", ".h", ".hpp") or "build" in path.parts:
            continue
        rel = path.relative_to(CODE).as_posix()
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for rx in (SEE_RE, MIRROR_RE):
            for m in rx.finditer(text):
                addr = m.group(1)
                e = entries[addr]
                e["files"].add(rel)

    # Dosya bağlamını son çare olarak ekle (ilk class tanımından)
    for addr, e in entries.items():
        if not e["descs"]:
            rel = sorted(e["files"])[0]
            try:
                text = (CODE / rel).read_text(encoding="utf-8", errors="replace")
            except OSError:
                text = ""
            e["descs"].add(file_context(rel, text))

    if not entries:
        print("FUN_ referansı bulunamadı", file=sys.stderr)
        return 1

    def sort_key(a):
        return a[4:]

    def best_desc(e):
        if e["methods"]:
            return "; ".join(sorted(e["methods"]))
        return sorted(e["descs"], key=len)[0]

    lines = [
        "# FUN_* → Sınıf::Metot Eşleme Tablosu",
        "",
        "> Faz 5.5 · Üreten: `python3 tools/extract_fun_map.py` (yeniden üretilebilir)",
        f"> Kaynak: `code/` yorumlarında geçen "
        f"**{len(entries)}** farklı `FUN_*` adresi.",
        "",
        "| FUN adresi | Eşleşen metot / açıklama | Kaynak dosya |",
        "|---|---|---|",
    ]
    for addr in sorted(entries, key=sort_key):
        e = entries[addr]
        desc = best_desc(e).replace("|", "\\|")
        files = ", ".join(sorted(e["files"])[:3])
        if len(e["files"]) > 3:
            files += f" (+{len(e['files']) - 3})"
        lines.append(f"| `{addr}` | {desc} | `{files}` |")

    matched = sum(1 for e in entries.values() if e["methods"])
    explicit = sum(
        1 for e in entries.values()
        if e["methods"] or not any("(dosya:" in d for d in e["descs"])
    )
    lines += [
        "",
        "## Özet",
        "",
        f"- Toplam referans: **{len(entries)}**",
        f"- `Class::Method` olarak eşleşen: **{matched}**",
        f"- Açıklayıcı metinle eşleşen (toplam): **{explicit}**",
        f"- Yalnızca dosya bağlamı: **{len(entries) - explicit}**",
        "",
        "*Tablo, kod yorumlarındaki `FUN_*` referanslarından üretilir; "
        "binary'de bulunan ancak henüz koda bağlanmamış FUN adreslerini "
        "kapsamaz (bkz. `CC.asm`).*",
        "",
    ]
    OUT.write_text("\n".join(lines), encoding="utf-8")
    print(f"{OUT.name}: {len(entries)} FUN adresi, {matched} metot eşleşmesi")
    return 0


if __name__ == "__main__":
    sys.exit(main())
