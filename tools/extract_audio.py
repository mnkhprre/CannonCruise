#!/usr/bin/env python3
"""
CannonCruise - RWS Ses Cikarici (RenderWare Audio Stream Extractor)
==================================================================
Bu arac, CannonCruise oyununun Sound/Win32 klasorundeki .rws (RenderWare Audio Stream)
dosyalarindan oyun muziklerini ve tum dillerdeki seslendirme / diyalog dosyalarini
kayipsiz WAV formatinda disa aktarir.

Kullanim:
    python tools/extract_audio.py
    python tools/extract_audio.py --output Sound/Extracted
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


def sanitize_filename(name: str) -> str:
    invalid_chars = '<>:"/\\|?*'
    for ch in invalid_chars:
        name = name.replace(ch, '_')
    return name.strip()


def find_vgmstream(repo_root: Path) -> Path:
    local_path = repo_root / "tools" / "vgmstream" / "vgmstream-cli.exe"
    if local_path.is_file():
        return local_path
    
    which_path = shutil.which("vgmstream-cli")
    if which_path:
        return Path(which_path)
    
    raise FileNotFoundError(
        f"vgmstream-cli.exe bulunamadi! Beklenen konum: {local_path}"
    )


def extract_music(vgmstream: Path, sound_dir: Path, output_dir: Path):
    music_out = output_dir / "Music"
    music_out.mkdir(parents=True, exist_ok=True)
    
    music_files = [
        "BeaverM.rws",
        "IdunaM.rws",
        "MenuMus.rws",
        "NordicM.rws",
        "VikingM.rws",
    ]
    
    print("\n[1/2] Oyun Muzikleri Cikariliyor...")
    print("-" * 50)
    
    for mf in music_files:
        in_file = sound_dir / mf
        if not in_file.is_file():
            print(f"  [UYARI] {mf} bulunamadi, atlaniyor.")
            continue
        
        out_name = f"{in_file.stem}.wav"
        out_file = music_out / out_name
        
        cmd = [str(vgmstream), "-i", "-o", str(out_file), str(in_file)]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        
        if res.returncode == 0 and out_file.is_file():
            size_mb = out_file.stat().st_size / (1024 * 1024)
            print(f"  -> {mf:15} => Music/{out_name:15} ({size_mb:.2f} MB)")
        else:
            print(f"  [HATA] {mf} cikarilamadi: {res.stderr.strip()}")


def extract_voices(vgmstream: Path, speaks_dir: Path, output_dir: Path):
    speaks_out = output_dir / "Speaks"
    speaks_out.mkdir(parents=True, exist_ok=True)
    
    # Desteklenen tum diller (Tyr = Turkce)
    languages = ["Tyr", "Den", "Eng", "Fin", "Nor", "Swe"]
    
    print("\n[2/2] Seslendirme / Diyalog Dosyalari Cikariliyor...")
    print("-" * 50)
    
    temp_dir = output_dir / "_temp_speaks"
    if temp_dir.exists():
        shutil.rmtree(temp_dir)
    temp_dir.mkdir(parents=True, exist_ok=True)
    
    for lang in languages:
        rws_file = speaks_dir / f"{lang}.rws"
        dat_file = speaks_dir / f"{lang}.dat"
        
        if not rws_file.is_file():
            # Kucuk/buyuk harf kontrolu
            found = list(speaks_dir.glob(f"{lang.lower()}.rws")) + list(speaks_dir.glob(f"{lang.upper()}.rws"))
            if found:
                rws_file = found[0]
            else:
                print(f"  [UYARI] {lang}.rws bulunamadi, atlaniyor.")
                continue
        
        cue_names = []
        if dat_file.is_file():
            with open(dat_file, "r", encoding="latin-1") as f:
                cue_names = [line.strip() for line in f if line.strip()]
        
        lang_out = speaks_out / lang
        lang_out.mkdir(parents=True, exist_ok=True)
        
        lang_label = f"{lang} (Turkce)" if lang.lower() == "tyr" else lang
        print(f"  -> {lang_label} isleniyor (Hedef: Speaks/{lang}/)...")
        
        pattern = temp_dir / f"{lang}_?s.wav"
        cmd = [str(vgmstream), "-i", "-s", "1", "-S", "0", "-o", str(pattern), str(rws_file)]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        
        if res.returncode != 0:
            print(f"  [HATA] {lang}.rws cikarilirken hata olustu: {res.stderr.strip()}")
            continue
        
        count = 0
        total_size = 0
        for i in range(1, len(cue_names) + 1):
            temp_file = temp_dir / f"{lang}_{i}.wav"
            if not temp_file.is_file():
                continue
            
            cue_label = sanitize_filename(cue_names[i - 1]) if i - 1 < len(cue_names) else f"cue_{i}"
            final_name = f"{i:03d}_{cue_label}.wav"
            final_file = lang_out / final_name
            
            shutil.move(str(temp_file), str(final_file))
            total_size += final_file.stat().st_size
            count += 1
            
        print(f"     Tamamlandi: {count} ses klibi aktarildi ({total_size / (1024 * 1024):.2f} MB)")
    
    if temp_dir.exists():
        shutil.rmtree(temp_dir)


def main():
    parser = argparse.ArgumentParser(
        description="CannonCruise .rws ses dosyalarini WAV formatinda disa aktarir."
    )
    repo_root = Path(__file__).resolve().parent.parent
    default_sound = repo_root / "Sound" / "Win32"
    default_output = repo_root / "Sound" / "Extracted"
    
    parser.add_argument("--sound-dir", type=Path, default=default_sound, help="Sound/Win32 dizini")
    parser.add_argument("--output", type=Path, default=default_output, help="Cikti dizini")
    args = parser.parse_args()
    
    vgmstream = find_vgmstream(repo_root)
    sound_dir = args.sound_dir
    speaks_dir = sound_dir / "Speaks"
    output_dir = args.output
    
    print("=" * 60)
    print("CannonCruise RWS Ses Dosyasi Cikarici")
    print("=" * 60)
    print(f"vgmstream : {vgmstream}")
    print(f"Kaynak    : {sound_dir}")
    print(f"Hedef     : {output_dir}")
    
    extract_music(vgmstream, sound_dir, output_dir)
    extract_voices(vgmstream, speaks_dir, output_dir)
    
    print("\n" + "=" * 60)
    print(f"[BASARILI] Tum sesler basariyla cikarildi: {output_dir}")
    print("=" * 60)


if __name__ == "__main__":
    main()
