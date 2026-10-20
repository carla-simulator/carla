import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 470
s = Svg(W, H)
s.text(W / 2, 44, "How CARLA content changes reach users", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "carla-content (Bitbucket, Git LFS), branch ue58-dev-carla next to the code branch ue58-dev",
       size=15, color=C["sub"], anchor="middle")
bw, gap = 214, 18
# change lane
s.rect(30, 95, 1200, 165, fill=C["card"], stroke=C["line"], r=14)
s.text(55, 125, "A content change", size=18, weight="bold")
change = [("Branch", ["<name>/<topic> from", "ue58-dev-carla"], "blue"),
          ("Edit in the editor", ["assets under", "Unreal/CarlaUnreal/Content/Carla"], "blue"),
          ("Push", ["commit and push the", "branch (LFS objects too)"], "blue"),
          ("Pull request", ["on Bitbucket, against", "ue58-dev-carla, reviewed"], "orange"),
          ("Merged", ["the tip of ue58-dev-carla", "moves forward"], "green")]
for i, (t, ls, col) in enumerate(change):
    x = 50 + i * (bw + gap)
    s.box(x, 140, bw, 100, t, ls, fill=C["soft_" + col], stroke=C[col], tsize=16, lsize=12)
    if i:
        s.line(x - gap + 2, 190, x - 3, 190, color=C["sub"], sw=2.5, arrow="sub")
# use lane
s.rect(30, 290, 1200, 165, fill=C["card"], stroke=C["line"], r=14)
s.text(1205, 320, "Who gets it", size=18, weight="bold", anchor="end")
use = [("New source checkout", ["CarlaSetup clones the tip", "of ue58-dev-carla"], "green"),
       ("Existing checkout", ["git pull in Content/Carla;", "setup does not update it"], "orange"),
       ("Release", ["both repos tagged with", "the version (e.g. 0.10.0)"], "purple"),
       ("CARLA package", ["cooks the content checked", "out when it is built"], "purple"),
       ("Content pack", ["ships extra maps or assets", "without a new package"], "purple")]
for i, (t, ls, col) in enumerate(use):
    x = 50 + i * (bw + gap)
    s.box(x, 335, bw, 100, t, ls, fill=C["soft_" + col], stroke=C[col], tsize=16, lsize=12)
    if i in (2, 3):
        s.line(x + bw + 2, 385, x + bw + gap - 3, 385, color=C["sub"], sw=2.5, arrow="sub")
# merged feeds the use lane
tx = 50 + 4 * (bw + gap) + bw / 2
for k in (0, 1, 2):
    ix = 50 + k * (bw + gap) + bw / 2
    yy = 268 + k * 6
    s.path(f"M{tx},240 L{tx},{yy} L{ix},{yy} L{ix},332", color=C["green"], sw=2, arrow="green")
out = sys.argv[1]
s.save(out + '/content_flow.svg')
render_png(out + '/content_flow.svg', out + '/content_flow.png', W, H)
