#!/usr/bin/env python3
"""Convert a CARLA town's OpenDRIVE into an Autoware-consumable lanelet2 .osm.

Input is either ``--xodr <file>`` (offline) or a running CARLA server
(``--host/--port``), in which case the map is pulled live via
``world.get_map().to_opendrive()``.

Engine: commonroad-scenario-designer (the maintained successor of the
opendrive2lanelet lineage), pinned in requirements.txt. It is configured with
``autoware=True`` + ``use_local_coordinates=True`` so every node carries
``local_x``/``local_y`` tags -- exactly what Autoware's *Local* map projector
reads (see map_projector_info.yaml).

Coordinate convention: OpenDRIVE is right-handed while CARLA is left-handed;
CARLA negates y when loading the xodr. The lanelet2 map therefore comes out
"y-inverted relative to CARLA" simply by converting the xodr as-is -- that IS
the convention the prebuilt autoware-contents maps use and what Autoware
expects. Do NOT flip the converter output. Live CARLA actor coordinates
(traffic lights, waypoints) ARE flipped (y := -y) before injection.

Traffic lights: the upstream prebuilt maps lack traffic-light regulatory
elements. When a live server is available this tool injects them from ground
truth: each ``traffic.traffic_light*`` actor contributes
* a ``type=traffic_light`` way per light box (``get_light_boxes()``),
* a ``type=stop_line`` way from ``get_stop_waypoints()``,
* a ``type=regulatory_element, subtype=traffic_light`` relation, referenced by
  every lanelet hit by ``get_affected_lane_waypoints()``.
Offline (--xodr) mode skips injection with a warning.
"""

from __future__ import annotations

import argparse
import math
import os
import sys
import tempfile
import xml.etree.ElementTree as ET

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from carla_common import carla_xyz_to_map, connect  # noqa: E402

CONVERTER_HELP = (
    "ERROR: commonroad-scenario-designer is not importable.\n"
    "Install the pinned tooling deps into a virtualenv, e.g.:\n"
    "  python3 -m venv ~/.venvs/carla-map-tools\n"
    "  ~/.venvs/carla-map-tools/bin/pip install -r Examples/av_stacks/autoware/map_tools/requirements.txt\n"
    "and run this script with that interpreter.\n"
)


def parse_args(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = p.add_argument_group("input (choose --xodr for offline mode, otherwise a live server is used)")
    src.add_argument("--xodr", help="OpenDRIVE file to convert offline (skips traffic-light injection)")
    src.add_argument("--host", default="127.0.0.1", help="CARLA server host (default: %(default)s)")
    src.add_argument("--port", type=int, default=2000, help="CARLA RPC port (default: %(default)s)")
    p.add_argument("--out", default="lanelet2_map.osm", help="output .osm path (default: %(default)s)")
    p.add_argument("--keep-xodr", metavar="PATH",
                   help="also save the (live-fetched) OpenDRIVE string to PATH")
    p.add_argument("--no-traffic-lights", action="store_true",
                   help="skip ground-truth traffic-light regulatory element injection")
    return p.parse_args(argv)


def ensure_metainfo(osm_path):
    """Guarantee ``<MetaInfo format_version="1.0.0" map_version="1"/>`` under ``<osm>``.

    Autoware's route_handler treats a lanelet2 map without this element as
    invalid: planning silently never starts and mission_planner can even
    segfault. The crdesigner converter does not emit it, so it is injected here
    as the first child of the root. Idempotent.
    """
    tree = ET.parse(osm_path)
    root = tree.getroot()
    if root.find("MetaInfo") is not None:
        return
    root.insert(0, ET.Element("MetaInfo", {"format_version": "1.0.0", "map_version": "1"}))
    ET.indent(tree, space="  ")
    tree.write(osm_path, encoding="UTF-8", xml_declaration=True)
    print(f"Injected <MetaInfo format_version=\"1.0.0\" map_version=\"1\"/> into {osm_path}",
          flush=True)


def convert_xodr_to_lanelet2(xodr_path, out_osm):
    """Run the crdesigner OpenDRIVE -> lanelet2 conversion (no y flip, see module doc)."""
    try:
        from crdesigner.common.config.lanelet2_config import lanelet2_config  # noqa: PLC0415
        from crdesigner.map_conversion.map_conversion_interface import opendrive_to_lanelet  # noqa: PLC0415
    except ImportError:
        sys.stderr.write(CONVERTER_HELP)
        sys.exit(2)
    lanelet2_config.autoware = True
    lanelet2_config.use_local_coordinates = True
    print(f"Converting {xodr_path} -> {out_osm} (crdesigner, autoware/local mode)...", flush=True)
    opendrive_to_lanelet(xodr_path, out_osm, lanelet2_config=lanelet2_config)
    if not os.path.isfile(out_osm) or os.path.getsize(out_osm) == 0:
        raise RuntimeError("conversion produced no output -- check the OpenDRIVE input")
    ensure_metainfo(out_osm)  # required by Autoware's route_handler
    print("Conversion done.", flush=True)


# --------------------------------------------------------------------------
# Traffic-light regulatory element injection (live server only)
# --------------------------------------------------------------------------

def _node_xy(node):
    lx = ly = ele = None
    for tag in node.findall("tag"):
        k = tag.get("k")
        if k == "local_x":
            lx = float(tag.get("v"))
        elif k == "local_y":
            ly = float(tag.get("v"))
        elif k == "ele":
            ele = float(tag.get("v"))
    return lx, ly, ele


class OsmMap:
    """Small mutable view over a converter-produced lanelet2 osm file."""

    def __init__(self, path):
        self.tree = ET.parse(path)
        self.root = self.tree.getroot()
        self.nodes = {n.get("id"): n for n in self.root.findall("node")}
        self.ways = {w.get("id"): w for w in self.root.findall("way")}
        ids = [int(e.get("id")) for e in self.root.iter() if e.get("id", "").lstrip("-").isdigit()]
        self._next_id = max(ids, default=0) + 1
        # lanelet relation -> Nx2 array of boundary node coords (map frame, meters)
        self.lanelets = []
        for rel in self.root.findall("relation"):
            tags = {t.get("k"): t.get("v") for t in rel.findall("tag")}
            if tags.get("type") != "lanelet":
                continue
            pts = []
            for member in rel.findall("member"):
                if member.get("type") != "way" or member.get("role") not in ("left", "right"):
                    continue
                way = self.ways.get(member.get("ref"))
                if way is None:
                    continue
                for nd in way.findall("nd"):
                    node = self.nodes.get(nd.get("ref"))
                    if node is None:
                        continue
                    lx, ly, _ = _node_xy(node)
                    if lx is not None and ly is not None:
                        pts.append((lx, ly))
            if pts:
                self.lanelets.append((rel, pts))

    def new_id(self):
        i = self._next_id
        self._next_id += 1
        return str(i)

    def add_node(self, x, y, z):
        nid = self.new_id()
        node = ET.SubElement(self.root, "node", {
            "id": nid, "action": "modify", "visible": "true", "version": "1",
            # lat/lon are placeholders: the Autoware *Local* projector reads
            # local_x/local_y only. Regenerate with a real projection for MGRS.
            "lat": "0.0", "lon": "0.0",
        })
        ET.SubElement(node, "tag", {"k": "local_x", "v": f"{x:.4f}"})
        ET.SubElement(node, "tag", {"k": "local_y", "v": f"{y:.4f}"})
        ET.SubElement(node, "tag", {"k": "ele", "v": f"{z:.4f}"})
        return nid

    def add_way(self, node_ids, tags):
        wid = self.new_id()
        way = ET.SubElement(self.root, "way", {
            "id": wid, "action": "modify", "visible": "true", "version": "1",
        })
        for nid in node_ids:
            ET.SubElement(way, "nd", {"ref": nid})
        for k, v in tags.items():
            ET.SubElement(way, "tag", {"k": k, "v": str(v)})
        return wid

    def nearest_lanelet(self, x, y, max_dist=6.0):
        best, best_d = None, max_dist
        for rel, pts in self.lanelets:
            for px, py in pts:
                d = math.hypot(px - x, py - y)
                if d < best_d:
                    best, best_d = rel, d
        return best

    def write(self, path):
        ET.indent(self.tree, space="  ")
        self.tree.write(path, encoding="UTF-8", xml_declaration=True)


def inject_traffic_lights(world, osm_path):
    osm = OsmMap(osm_path)
    if not osm.lanelets:
        print("WARNING: no lanelets with local_x/local_y found; skipping traffic-light injection.",
              flush=True)
        return 0
    lights = list(world.get_actors().filter("traffic.traffic_light*"))
    print(f"Injecting regulatory elements for {len(lights)} traffic lights...", flush=True)
    injected = 0
    for tl in lights:
        light_way_ids = []
        for bb in tl.get_light_boxes():
            right = bb.rotation.get_right_vector()
            cx, cy, cz = bb.location.x, bb.location.y, bb.location.z
            ey, ez = bb.extent.y, bb.extent.z
            bottom = cz - ez
            p1 = carla_xyz_to_map(cx - right.x * ey, cy - right.y * ey, bottom)
            p2 = carla_xyz_to_map(cx + right.x * ey, cy + right.y * ey, bottom)
            n1 = osm.add_node(*p1)
            n2 = osm.add_node(*p2)
            light_way_ids.append(osm.add_way([n1, n2], {
                "type": "traffic_light",
                "subtype": "red_yellow_green",
                "height": f"{2.0 * ez:.3f}",
            }))
        if not light_way_ids:
            continue

        stop_way_id = None
        stop_wps = tl.get_stop_waypoints()
        if stop_wps:
            endpoints = []
            for wp in stop_wps:
                tf = wp.transform
                r = tf.rotation.get_right_vector()
                half = 0.5 * wp.lane_width
                loc = tf.location
                endpoints.append(carla_xyz_to_map(loc.x - r.x * half, loc.y - r.y * half, loc.z))
                endpoints.append(carla_xyz_to_map(loc.x + r.x * half, loc.y + r.y * half, loc.z))
            # Single 2-point way spanning all affected lanes: take the two
            # extreme endpoints along the lateral axis of the first waypoint.
            r0 = stop_wps[0].transform.rotation.get_right_vector()
            axis = (r0.x, -r0.y)  # lateral axis, already in map frame
            endpoints.sort(key=lambda p: p[0] * axis[0] + p[1] * axis[1])
            a, b = endpoints[0], endpoints[-1]
            stop_way_id = osm.add_way(
                [osm.add_node(*a), osm.add_node(*b)], {"type": "stop_line"})

        re_id = osm.new_id()
        rel = ET.SubElement(osm.root, "relation", {
            "id": re_id, "action": "modify", "visible": "true", "version": "1",
        })
        for wid in light_way_ids:
            ET.SubElement(rel, "member", {"type": "way", "ref": wid, "role": "refers"})
        if stop_way_id is not None:
            ET.SubElement(rel, "member", {"type": "way", "ref": stop_way_id, "role": "ref_line"})
        ET.SubElement(rel, "tag", {"k": "type", "v": "regulatory_element"})
        ET.SubElement(rel, "tag", {"k": "subtype", "v": "traffic_light"})

        # Reference the regulatory element from every affected lanelet.
        linked = set()
        for wp in tl.get_affected_lane_waypoints():
            loc = wp.transform.location
            mx, my, _ = carla_xyz_to_map(loc.x, loc.y, loc.z)
            lanelet_rel = osm.nearest_lanelet(mx, my)
            if lanelet_rel is None or id(lanelet_rel) in linked:
                continue
            linked.add(id(lanelet_rel))
            ET.SubElement(lanelet_rel, "member", {
                "type": "relation", "ref": re_id, "role": "regulatory_element",
            })
        injected += 1
    osm.write(osm_path)
    print(f"Injected {injected} traffic-light regulatory elements -> {osm_path}", flush=True)
    return injected


def generate(args, world=None):
    """Programmatic entry point (used by generate_map_artifacts.py)."""
    out_dir = os.path.dirname(os.path.abspath(args.out))
    os.makedirs(out_dir, exist_ok=True)
    if args.xodr:
        xodr_path = args.xodr
        if not os.path.isfile(xodr_path):
            sys.stderr.write(f"ERROR: OpenDRIVE file not found: {xodr_path}\n")
            sys.exit(2)
        convert_xodr_to_lanelet2(xodr_path, args.out)
        if not args.no_traffic_lights:
            print("WARNING: offline (--xodr) mode -- traffic-light regulatory elements are NOT "
                  "injected (needs a live server for ground truth). The map will lack "
                  "traffic-light regulation, like the upstream prebuilt maps.", flush=True)
        return
    if world is None:
        _, world = connect(args.host, args.port)
    xodr_string = world.get_map().to_opendrive()
    if args.keep_xodr:
        with open(args.keep_xodr, "w", encoding="utf-8") as f:
            f.write(xodr_string)
        xodr_path = args.keep_xodr
    else:
        fd, xodr_path = tempfile.mkstemp(suffix=".xodr", prefix="carla_map_")
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            f.write(xodr_string)
    try:
        convert_xodr_to_lanelet2(xodr_path, args.out)
    finally:
        if not args.keep_xodr:
            os.unlink(xodr_path)
    if args.no_traffic_lights:
        print("Skipping traffic-light injection (--no-traffic-lights).", flush=True)
    else:
        inject_traffic_lights(world, args.out)


def main(argv=None):
    args = parse_args(argv)
    generate(args)
    print(f"Wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
