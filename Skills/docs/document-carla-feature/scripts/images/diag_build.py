import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 320
s = Svg(W, H)
s.text(W / 2, 44, "Building CARLA from source (Linux, UE 5.8)", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "What the one-time setup script CarlaSetup.sh does, in order",
       size=15, color=C["sub"], anchor="middle")
# setup column
s.rect(30, 95, 1200, 205, fill=C["card"], stroke=C["line"], r=14)
s.text(55, 127, "One-time setup (CarlaSetup.sh)", size=18, weight="bold")
steps = [("Prerequisites", ["system packages", "and Python modules"]),
         ("Content", ["Unreal/CarlaUnreal/Content/Carla", "branch ue58-dev-carla"]),
         ("DLSS SDK", ["NVIDIA SDK download", "(optional, can be disabled)"]),
         ("Unreal Engine 5.8", ["../UnrealEngine5_carla", "clone + build (hours)"]),
         ("Configure + build", ["CMake configure and build", "then Python API install"])]
bw, gap = 214, 18
for i, (t, ls) in enumerate(steps):
    x = 50 + i * (bw + gap)
    s.box(x, 145, bw, 130, t, ls, fill=C["soft_blue"] if i < 4 else C["soft_green"],
          stroke=C["blue"] if i < 4 else C["green"], tsize=16, lsize=12)
    if i:
        s.line(x - gap + 2, 210, x - 3, 210, color=C["sub"], sw=2.5, arrow="sub")
out = sys.argv[1]
s.save(out + '/build_pipeline_linux.svg')
render_png(out + '/build_pipeline_linux.svg', out + '/build_pipeline_linux.svg'.replace('.svg', '.png'), W, H)
