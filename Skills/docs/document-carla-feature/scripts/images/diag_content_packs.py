import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 470
s = Svg(W, H)
s.text(W / 2, 44, "Content pack: from a source build to a CARLA package", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "Built once against a CARLA release, installed into any package of that release",
       size=15, color=C["sub"], anchor="middle")
# making lane
s.rect(30, 95, 1200, 165, fill=C["card"], stroke=C["line"], r=14)
s.text(55, 125, "Making a pack (CARLA source build with the editor)", size=18, weight="bold")
make = [("Release metadata", ["base asset registry, written", "by the package target"], "purple"),
        ("Author", ["maps, props, vehicles, walkers", "in the pack's own folder"], "blue"),
        ("Register", ["init, add (or create)", "manifest, catalogs, sidecars"], "blue"),
        ("Build", ["DLC cook against", "the release metadata"], "blue"),
        ("Pack tarball", ["<Pack>-<version>-<release>", ".tar.gz"], "green")]
bw, gap = 214, 18
for i, (t, ls, col) in enumerate(make):
    x = 50 + i * (bw + gap)
    s.box(x, 140, bw, 100, t, ls, fill=C["soft_" + col], stroke=C[col], tsize=16, lsize=12)
    if 1 < i:
        s.line(x - gap + 2, 190, x - 3, 190, color=C["sub"], sw=2.5, arrow="sub")
# metadata feeds the build (curved, below the boxes)
mx = 50 + bw / 2
bx = 50 + 3 * (bw + gap) + bw / 2
s.path(f"M{mx},240 C{mx},258 {bx},258 {bx},243", color=C["purple"], sw=2, arrow="purple", dash="6,4")
# installing lane
s.rect(30, 290, 1200, 165, fill=C["card"], stroke=C["line"], r=14)
s.text(1205, 320, "Installing a pack (CARLA package only)", size=18, weight="bold", anchor="end")
use = [("Install", ["checksums, release and", "platform checked"], "green"),
       ("Linux/CarlaUnreal/Packs", ["or -carla-packs, $CARLA_PACKS", "read when the server starts"], "orange"),
       ("Server mounts it", ["maps listed, actors in the", "blueprint library"], "orange"),
       ("Client", ["load_world(map)", "spawn the pack's actors"], "orange"),
       ("Or at run time", ["mount_content_pack(path)", "on a running server"], "orange")]
for i, (t, ls, col) in enumerate(use):
    x = 50 + i * (bw + gap)
    s.box(x, 335, bw, 100, t, ls, fill=C["soft_" + col], stroke=C[col], tsize=16, lsize=12)
    if 0 < i < 4:
        s.line(x - gap + 2, 385, x - 3, 385, color=C["sub"], sw=2.5, arrow="sub")
# run-time mount goes into the server box
rx = 50 + 4 * (bw + gap) + bw / 2
sx = 50 + 2 * (bw + gap) + bw / 2
s.path(f"M{rx},435 C{rx},458 {sx},458 {sx},438", color=C["orange"], sw=2, arrow="orange", dash="6,4")
# tarball goes to install
tx = 50 + 4 * (bw + gap) + bw / 2
ix = 50 + bw / 2
s.path(f"M{tx},240 L{tx},266 Q{tx},275 {tx-9},275 L{ix+9},275 Q{ix},275 {ix},284 L{ix},332", color=C["green"], sw=2.5, arrow="green")
out = sys.argv[1]
s.save(out + '/content_pack_lifecycle.svg')
render_png(out + '/content_pack_lifecycle.svg', out + '/content_pack_lifecycle.png', W, H)
