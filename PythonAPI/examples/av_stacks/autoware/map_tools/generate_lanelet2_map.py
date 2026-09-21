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
  every approach lane identified by ``get_stop_waypoints()``.
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
    "  ~/.venvs/carla-map-tools/bin/pip install -r PythonAPI/examples/av_stacks/autoware/map_tools/requirements.txt\n"
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
    prune_untyped_lanelets(out_osm)
    print("Conversion done.", flush=True)


def prune_untyped_lanelets(osm_path):
    """Remove lanelet relations that carry no subtype.

    crdesigner emits border/median lanes of the OpenDRIVE road as bare
    ``type=lanelet`` relations without a subtype. lanelet2's vehicle traffic
    rules treat them as routable, but their centerlines sit meters off any
    real driving lane, so a route through one steers the vehicle off the
    road. Every driving lane gets ``subtype=road`` from the converter, so
    dropping untyped lanelets never breaks lane connectivity."""
    tree = ET.parse(osm_path)
    root = tree.getroot()
    pruned = 0
    for rel in list(root.findall("relation")):
        tags = {t.get("k"): t.get("v") for t in rel.findall("tag")}
        if tags.get("type") == "lanelet" and "subtype" not in tags:
            root.remove(rel)
            pruned += 1
    if pruned:
        tree.write(osm_path, encoding="UTF-8", xml_declaration=True)
    print(f"Pruned {pruned} untyped (non-driving) lanelet relations.", flush=True)


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


def _check_unique_opendrive_id(kind, actor_id, opendrive_id, seen_ids):
    """Validate ``opendrive_id`` for the deterministic id scheme: numeric, and
    not already claimed by another actor of the same kind this run."""
    try:
        # The id scheme keys on int(opendrive_id), so "007" and "7" must
        # collide here too, or two actors can silently emit the same
        # regulatory-element id.
        numeric_id = int(opendrive_id)
    except ValueError:
        raise ValueError(
            f"{kind} actor {actor_id} has non-numeric OpenDRIVE id {opendrive_id!r}; "
            "this map-generation tool's deterministic id scheme currently requires "
            "numeric OpenDRIVE ids."
        )
    if numeric_id in seen_ids:
        raise RuntimeError(
            f"{kind} actors {seen_ids[numeric_id]} and {actor_id} both report OpenDRIVE "
            f"id {opendrive_id!r}; the deterministic id scheme requires each {kind} to have "
            "a unique OpenDRIVE id."
        )
    seen_ids[numeric_id] = actor_id


# Ids for injected elements are derived deterministically from each actor's
# OpenDRIVE id instead of a running counter, so re-running the injectors
# against an unchanged town regenerates byte-identical ids. Each element kind
# gets its own base, far above anything crdesigner emits, and a stride far
# above the handful of nodes/ways/relations one actor's injection creates.
TRAFFIC_LIGHT_ID_BASE = 900_000_000
TRAFFIC_LIGHT_ID_STRIDE = 1000


class OsmMap:
    """Small mutable view over a converter-produced lanelet2 osm file."""

    def __init__(self, path):
        self.tree = ET.parse(path)
        self.root = self.tree.getroot()
        self.nodes = {n.get("id"): n for n in self.root.findall("node")}
        self.ways = {w.get("id"): w for w in self.root.findall("way")}
        # lanelet relation -> closed boundary polygon (map frame, meters)
        self.lanelets = []
        for rel in self.root.findall("relation"):
            tags = {t.get("k"): t.get("v") for t in rel.findall("tag")}
            if tags.get("type") != "lanelet":
                continue
            boundaries = {}
            for member in rel.findall("member"):
                if member.get("type") != "way" or member.get("role") not in ("left", "right"):
                    continue
                way = self.ways.get(member.get("ref"))
                if way is None:
                    continue
                pts = []
                for nd in way.findall("nd"):
                    node = self.nodes.get(nd.get("ref"))
                    if node is None:
                        continue
                    lx, ly, _ = _node_xy(node)
                    if lx is not None and ly is not None:
                        pts.append((lx, ly))
                if pts:
                    boundaries[member.get("role")] = pts
            if "left" in boundaries and "right" in boundaries:
                left_pts, right_pts = boundaries["left"], boundaries["right"]
                polygon = left_pts + list(reversed(right_pts))
                # Containment assumes left/right boundaries were sampled in
                # lockstep; a mismatch would fold the polygon on itself, so
                # such a lanelet is excluded from containment (but its edges
                # are still usable for the distance fallback below).
                containment_ok = len(left_pts) == len(right_pts)
                if not containment_ok:
                    print(f"WARNING: lanelet {rel.get('id')} has mismatched left/right "
                          f"boundary point counts ({len(left_pts)} vs {len(right_pts)}); "
                          "excluding it from polygon-containment matching.", flush=True)
                self.lanelets.append((rel, polygon, containment_ok))

    def traffic_light_id(self, opendrive_id, slot):
        return str(TRAFFIC_LIGHT_ID_BASE + int(opendrive_id) * TRAFFIC_LIGHT_ID_STRIDE + slot)

    def add_node(self, x, y, z, node_id):
        if node_id in self.nodes:
            raise RuntimeError(
                f"node id {node_id!r} already exists in the map; the deterministic id scheme "
                "has collided with another element (id-band overflow or a stale leftover node)."
            )
        node = ET.SubElement(self.root, "node", {
            "id": node_id, "action": "modify", "visible": "true", "version": "1",
            # lat/lon are placeholders: the Autoware *Local* projector reads
            # local_x/local_y only. Regenerate with a real projection for MGRS.
            "lat": "0.0", "lon": "0.0",
        })
        ET.SubElement(node, "tag", {"k": "local_x", "v": f"{x:.4f}"})
        ET.SubElement(node, "tag", {"k": "local_y", "v": f"{y:.4f}"})
        ET.SubElement(node, "tag", {"k": "ele", "v": f"{z:.4f}"})
        self.nodes[node_id] = node
        return node_id

    def add_way(self, node_ids, tags, way_id):
        if way_id in self.ways:
            raise RuntimeError(
                f"way id {way_id!r} already exists in the map; the deterministic id scheme "
                "has collided with another element (id-band overflow or a stale leftover way)."
            )
        way = ET.SubElement(self.root, "way", {
            "id": way_id, "action": "modify", "visible": "true", "version": "1",
        })
        for nid in node_ids:
            ET.SubElement(way, "nd", {"ref": nid})
        for k, v in tags.items():
            ET.SubElement(way, "tag", {"k": k, "v": str(v)})
        self.ways[way_id] = way
        return way_id

    def nearest_lanelet(self, x, y, max_dist=6.0):
        # max_dist (m): cutoff for a stop/affected waypoint that isn't
        # strictly inside any lanelet polygon.
        # CARLA returns affected waypoints on lane centerlines. Prefer polygon
        # containment: measuring only to boundary vertices can select the lane
        # on the other side of a shared boundary, especially on long segments.
        containing = [rel for rel, polygon, containment_ok in self.lanelets
                      if containment_ok and self._point_in_polygon(x, y, polygon)]
        if len(containing) == 1:
            return containing[0]

        best, best_d = None, max_dist
        # With zero or several containing lanelets, narrow the edge-distance
        # search to just those (an empty `containing` searches everything).
        candidates = ((rel, polygon) for rel, polygon, _ in self.lanelets
                      if not containing or rel in containing)
        for rel, polygon in candidates:
            for i, a in enumerate(polygon):
                b = polygon[(i + 1) % len(polygon)]
                d = self._point_segment_distance(x, y, a, b)
                if d < best_d:
                    best, best_d = rel, d
        return best

    @staticmethod
    def _point_in_polygon(x, y, polygon):
        inside = False
        j = len(polygon) - 1
        for i, (xi, yi) in enumerate(polygon):
            xj, yj = polygon[j]
            if ((yi > y) != (yj > y)):
                x_cross = (xj - xi) * (y - yi) / (yj - yi) + xi
                if x < x_cross:
                    inside = not inside
            j = i
        return inside

    @staticmethod
    def _point_segment_distance(x, y, a, b):
        dx, dy = b[0] - a[0], b[1] - a[1]
        length_sq = dx * dx + dy * dy
        if length_sq == 0.0:
            return math.hypot(x - a[0], y - a[1])
        u = max(0.0, min(1.0, ((x - a[0]) * dx + (y - a[1]) * dy) / length_sq))
        return math.hypot(x - (a[0] + u * dx), y - (a[1] + u * dy))

    def write(self, path):
        ET.indent(self.tree, space="  ")
        self.tree.write(path, encoding="UTF-8", xml_declaration=True)


def _way_still_referenced(root, way_id):
    return any(
        member.get("type") == "way" and member.get("ref") == way_id
        for rel in root.findall("relation")
        for member in rel.findall("member")
    )


def _node_still_referenced(root, ways, node_id):
    if any(nd.get("ref") == node_id for way in ways.values() for nd in way.findall("nd")):
        return True
    return any(
        member.get("type") == "node" and member.get("ref") == node_id
        for rel in root.findall("relation")
        for member in rel.findall("member")
    )


def _remove_regulatory_elements(osm, matches):
    """Remove every regulatory-element relation for which ``matches(tags)`` is
    true, drop the lanelet back-references pointing at it, then drop any of
    its member ways/nodes nothing else in the map still references.

    Shared by every injector below: each re-run must clean up its own
    previous injection (or a live-authoritative element keeps regulating a
    lanelet after the actor it modeled disappears), and the converter itself
    can emit relations needing the same treatment before ground truth
    replaces them.
    """
    root = osm.root
    relation_ids = set()
    for rel in root.findall("relation"):
        tags = {t.get("k"): t.get("v") for t in rel.findall("tag")}
        if tags.get("type") == "regulatory_element" and matches(tags):
            relation_ids.add(rel.get("id"))
    if not relation_ids:
        return 0

    candidate_way_ids = set()
    for rel in root.findall("relation"):
        if rel.get("id") not in relation_ids:
            continue
        for member in rel.findall("member"):
            if member.get("type") == "way":
                candidate_way_ids.add(member.get("ref"))

    for rel in root.findall("relation"):
        for member in list(rel.findall("member")):
            if (member.get("type") == "relation" and
                    member.get("role") == "regulatory_element" and
                    member.get("ref") in relation_ids):
                rel.remove(member)
    for rel in list(root.findall("relation")):
        if rel.get("id") in relation_ids:
            root.remove(rel)

    stale_way_ids = {wid for wid in candidate_way_ids if not _way_still_referenced(root, wid)}
    stale_node_ids = set()
    for way_id in stale_way_ids:
        way = osm.ways.get(way_id)
        if way is None:
            continue
        stale_node_ids.update(nd.get("ref") for nd in way.findall("nd"))
        root.remove(way)
        del osm.ways[way_id]

    for node_id in stale_node_ids:
        if _node_still_referenced(root, osm.ways, node_id):
            continue
        node = osm.nodes.get(node_id)
        if node is None:
            continue
        root.remove(node)
        del osm.nodes[node_id]

    return len(relation_ids)


def inject_traffic_lights(world, osm_path):
    osm = OsmMap(osm_path)
    if not osm.lanelets:
        print("WARNING: no lanelets with local_x/local_y found; skipping traffic-light injection.",
              flush=True)
        return 0

    lights = list(world.get_actors().filter("traffic.traffic_light*"))
    if not lights:
        print("WARNING: no live CARLA traffic-light actors found; leaving the map unchanged.",
              flush=True)
        return 0

    removed = _remove_regulatory_elements(osm, lambda tags: tags.get("subtype") == "traffic_light")
    if removed:
        print(f"Removed {removed} converter/previous traffic-light regulatory elements.",
              flush=True)

    print(f"Injecting regulatory elements for {len(lights)} traffic lights...", flush=True)
    injected = 0
    seen_opendrive_ids = {}
    for tl in lights:
        opendrive_id = tl.get_opendrive_id()
        _check_unique_opendrive_id("traffic light", tl.id, opendrive_id, seen_opendrive_ids)
        tl_slot = 0

        def _tl_id():
            nonlocal tl_slot
            allocated = osm.traffic_light_id(opendrive_id, tl_slot)
            tl_slot += 1
            return allocated

        stop_wps = tl.get_stop_waypoints()
        affected_wps = tl.get_affected_lane_waypoints()
        approach_wps = stop_wps or affected_wps
        line_direction = None
        lateral_carla = None
        if approach_wps:
            # All boxes and the stop line below share one heading, taken from
            # the first approach waypoint. This assumes a single approach
            # direction per signal group; a group governing diverging lanes
            # (e.g. straight + turn) would need a heading per lane instead.
            forward = approach_wps[0].transform.rotation.get_forward_vector()
            # CARLA y is inverted in the map frame. Lanelet2 traffic-light
            # lines are directed so their +90-degree normal follows traffic.
            travel_x, travel_y = forward.x, -forward.y
            norm = math.hypot(travel_x, travel_y)
            if norm > 0.0:
                travel_x, travel_y = travel_x / norm, travel_y / norm
                line_direction = (travel_y, -travel_x)
                lateral_carla = (line_direction[0], -line_direction[1])

        light_way_ids = []
        for bb in tl.get_light_boxes():
            cx, cy, cz = bb.location.x, bb.location.y, bb.location.z
            ez = bb.extent.z
            bottom = cz - ez
            if line_direction is not None:
                box_forward = bb.rotation.get_forward_vector()
                box_right = bb.rotation.get_right_vector()
                half_width = (
                    abs(lateral_carla[0] * box_forward.x +
                        lateral_carla[1] * box_forward.y) * bb.extent.x +
                    abs(lateral_carla[0] * box_right.x +
                        lateral_carla[1] * box_right.y) * bb.extent.y
                )
                map_cx, map_cy, _ = carla_xyz_to_map(cx, cy, bottom)
                lx, ly = line_direction
                p1 = (map_cx - lx * half_width, map_cy - ly * half_width, bottom)
                p2 = (map_cx + lx * half_width, map_cy + ly * half_width, bottom)
            else:
                # Defensive fallback for maps whose traffic-light actors do not
                # expose affected or stop waypoints.
                right = bb.rotation.get_right_vector()
                ey = bb.extent.y
                p1 = carla_xyz_to_map(cx - right.x * ey, cy - right.y * ey, bottom)
                p2 = carla_xyz_to_map(cx + right.x * ey, cy + right.y * ey, bottom)
            n1 = osm.add_node(*p1, node_id=_tl_id())
            n2 = osm.add_node(*p2, node_id=_tl_id())
            light_way_ids.append(osm.add_way([n1, n2], {
                "type": "traffic_light",
                "subtype": "red_yellow_green",
                "height": f"{2.0 * ez:.3f}",
            }, way_id=_tl_id()))
        if not light_way_ids:
            continue

        stop_way_id = None
        if stop_wps:
            a, b = _lane_stop_line_endpoints(stop_wps)
            stop_way_id = osm.add_way(
                [osm.add_node(*a, node_id=_tl_id()), osm.add_node(*b, node_id=_tl_id())],
                {"type": "stop_line"}, way_id=_tl_id())

        re_id = _tl_id()
        rel = ET.SubElement(osm.root, "relation", {
            "id": re_id, "action": "modify", "visible": "true", "version": "1",
        })
        for wid in light_way_ids:
            ET.SubElement(rel, "member", {"type": "way", "ref": wid, "role": "refers"})
        if stop_way_id is not None:
            ET.SubElement(rel, "member", {"type": "way", "ref": stop_way_id, "role": "ref_line"})
        ET.SubElement(rel, "tag", {"k": "type", "v": "regulatory_element"})
        ET.SubElement(rel, "tag", {"k": "subtype", "v": "traffic_light"})
        ET.SubElement(rel, "tag", {"k": "carla_opendrive_id", "v": str(opendrive_id)})

        # Stop waypoints identify the approach lanes that own the stop line.
        # Affected waypoints may instead lie on junction connector lanelets,
        # which makes the signal invisible to a route approaching the stop.
        linked = set()
        for wp in approach_wps:
            loc = wp.transform.location
            mx, my, _ = carla_xyz_to_map(loc.x, loc.y, loc.z)
            lanelet_rel = osm.nearest_lanelet(mx, my)
            if lanelet_rel is None:
                print(f"WARNING: traffic light {opendrive_id} has no lanelet match for "
                      f"approach waypoint at map ({mx:.2f}, {my:.2f}); skipping this "
                      "association.", flush=True)
                continue
            if id(lanelet_rel) in linked:
                continue
            linked.add(id(lanelet_rel))
            ET.SubElement(lanelet_rel, "member", {
                "type": "relation", "ref": re_id, "role": "regulatory_element",
            })
        injected += 1
    osm.write(osm_path)
    print(f"Injected {injected} traffic-light regulatory elements -> {osm_path}", flush=True)
    return injected


def _lane_stop_line_endpoints(wps):
    """Two map-frame endpoints spanning ``wps``' lanes, perpendicular to the
    first waypoint's heading. Shared by every stop-line construction below."""
    endpoints = []
    for wp in wps:
        tf = wp.transform
        r = tf.rotation.get_right_vector()
        half = 0.5 * wp.lane_width
        loc = tf.location
        endpoints.append(carla_xyz_to_map(loc.x - r.x * half, loc.y - r.y * half, loc.z))
        endpoints.append(carla_xyz_to_map(loc.x + r.x * half, loc.y + r.y * half, loc.z))
    # Extreme points along the lateral axis span every lane in wps.
    r0 = wps[0].transform.rotation.get_right_vector()
    axis = (r0.x, -r0.y)  # already in map frame (y negated)
    endpoints.sort(key=lambda p: p[0] * axis[0] + p[1] * axis[1])
    return endpoints[0], endpoints[-1]


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
