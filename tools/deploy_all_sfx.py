import os
import shutil

src_sfx = r"c:\Users\mnkhprre\Documents\GitHub\CannonCruise\Sound\Extracted\SFX"
dst_sound = r"c:\Users\mnkhprre\Documents\GitHub\CannonCruise\Sound\Win32"
build_dirs = [
    r"c:\Users\mnkhprre\Documents\GitHub\CannonCruise\code\build_win32_d3d9\Release",
    r"c:\Users\mnkhprre\Documents\GitHub\CannonCruise\code\build_win32_v180\Release"
]

# Ensure Ship_Sink and Audio_GameOver exist
shutil.copy2(os.path.join(src_sfx, "WoodCracking.wav"), os.path.join(src_sfx, "Ship_Sink.wav"))
shutil.copy2(os.path.join(src_sfx, "WoodCracking.wav"), os.path.join(src_sfx, "Audio_GameOver.wav"))

# Copy all to Sound/Win32
for f in os.listdir(src_sfx):
    if f.endswith(".wav"):
        s = os.path.join(src_sfx, f)
        shutil.copy2(s, os.path.join(dst_sound, f))
        for b in build_dirs:
            if os.path.isdir(b):
                shutil.copy2(s, os.path.join(b, f))

print(f"Deployed {len(os.listdir(src_sfx))} SFX files across all targets!")
