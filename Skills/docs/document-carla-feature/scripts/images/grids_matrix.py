"""Weather and camera-profile grids from a looks.py --mode matrix capture. grids_matrix.py MATRIX_DIR FINAL_DIR TOWNTAG"""
import os, subprocess, sys
R, F, TAG = sys.argv[1:4]
W = ["ClearNoon", "CloudyNoon", "WetNoon", "MidRainyNoon", "HardRainNoon", "ClearSunset", "SoftRainSunset", "ClearNight", "HardRainNight"]
P = ["AutomotiveHDR", "AutomotiveHDRClean", "Cinematic", "GoPro", "Dashcam", "LegacyCCD", "LogHDR", "Smartphone", "WideHDR8MP"]
here = os.path.dirname(os.path.abspath(__file__))
def grid(out, items):
    os.makedirs(os.path.dirname(out), exist_ok=True)
    subprocess.check_call([sys.executable, os.path.join(here, "grid.py"), out, "3"] + ["%s:%s" % it for it in items if os.path.exists(it[0])])
for prof in ("AutomotiveHDR", "Cinematic"):
    grid(f"{F}/core_world/weather_presets_{prof.lower()}_{TAG}.webp", [(f"{R}/{w}/{prof}.png", w) for w in W])
for w in ("ClearNoon", "HardRainNoon", "ClearSunset", "ClearNight"):
    grid(f"{F}/camera_postprocess_presets/camera_profiles_{w.lower()}_{TAG}.webp", [(f"{R}/{w}/{p}.png", p) for p in P])
