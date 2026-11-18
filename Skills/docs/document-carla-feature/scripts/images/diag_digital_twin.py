import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
W, H = 1260, 500
s = Svg(W, H)
s.text(W / 2, 44, "Digital Twin Tool: stages in this release", size=26, weight="bold", anchor="middle")
s.text(W / 2, 72, "From OpenStreetMap data to a CARLA map, in the CARLA editor",
       size=15, color=C["sub"], anchor="middle")
bw, gap = 214, 18
# generation lane
s.rect(30, 95, 1200, 165, fill=C["card"], stroke=C["line"], r=14)
s.text(55, 125, "Generation (UW_OnRoadMainWidget, GenerateTile commandlet)", size=18, weight="bold")
gen = [("OSM data", ["URL from openstreetmap.org", "(Overpass API export)"], "blue"),
       ("OSM to OpenDRIVE", ["osm2odr converter", "not in the repository"], "red"),
       ("Roads and terrain", ["road meshes, lane marks,", "terrain"], "orange"),
       ("Curbs, sidewalks", ["detected from a render", "by Python scripts"], "orange"),
       ("Buildings", ["extruded from OSM", "footprints and heights"], "orange")]
for i, (t, ls, col) in enumerate(gen):
    x = 50 + i * (bw + gap)
    s.box(x, 140, bw, 100, t, ls, fill=C["soft_" + col], stroke=C[col], tsize=16, lsize=12)
    if i:
        s.line(x - gap + 2, 190, x - 3, 190, color=C["sub"], sw=2.5, arrow="sub")
# tools lane
s.rect(30, 290, 1200, 165, fill=C["card"], stroke=C["line"], r=14)
s.text(1205, 320, "Tools that work on a map (editor)", size=18, weight="bold", anchor="end")
tools = [("Traffic lights", ["Traffic Light Tool (meshes from the", "content); map_logic.json phases"], "orange"),
         ("Traffic signs", ["Sign Generation Controller,", "Europe / America / China"], "green"),
         ("Vegetation", ["Procedural Vegetation Tool", "(PCG, JSON plan)"], "green"),
         ("Street furniture", ["Procedural Furniture Tool", "(PCG, JSON plan)"], "green"),
         ("Twin map", ["load_world(name),", "or ship as a content pack"], "blue")]
for i, (t, ls, col) in enumerate(tools):
    x = 50 + i * (bw + gap)
    s.box(x, 335, bw, 100, t, ls, fill=C["soft_" + col], stroke=C[col], tsize=16, lsize=12)
    if i:
        s.line(x - gap + 2, 385, x - 3, 385, color=C["sub"], sw=2.5, arrow="sub")
# generation feeds the tools
tx = 50 + 4 * (bw + gap) + bw / 2
ix = 50 + bw / 2
s.path(f"M{tx},240 L{tx},266 Q{tx},275 {tx-9},275 L{ix+9},275 Q{ix},275 {ix},284 L{ix},332",
       color=C["sub"], sw=2.5, arrow="sub")
# legend
ly = 478
items = [("red", "not available"), ("orange", "needs the CarlaDigitalTwinsTool content"),
         ("green", "available in the editor"), ("blue", "input / result")]
x = 120
for col, label in items:
    s.rect(x, ly - 13, 18, 18, fill=C["soft_" + col], stroke=C[col], r=4, sw=2)
    s.text(x + 26, ly + 1, label, size=14, color=C["sub"])
    x += 60 + len(label) * 7.6
out = sys.argv[1]
s.save(out + '/digital_twin_tool.svg')
render_png(out + '/digital_twin_tool.svg', out + '/digital_twin_tool.png', W, H)
