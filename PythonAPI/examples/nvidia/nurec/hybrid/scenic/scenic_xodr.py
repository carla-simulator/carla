"""OpenDRIVE from a NuRec artifact -> a copy Scenic's parser accepts.

The exporter writes junction connections whose incoming road is ``-1`` (none) or a road that itself belongs to a junction,
and junctions that end up with no usable connection at all. CARLA's OpenDRIVE loader ignores those; Scenic's
``xodr_parser`` only accepts plain roads as incoming roads and orders the roads of every junction around their centroid,
dividing by their number, which is zero for such a junction. This module drops the invalid connections and *dissolves*
every junction that keeps none: its connecting roads become plain roads and the neighbours' junction links are rewritten
to point at them. A dissolved junction can make a road plain that another junction uses as its incoming road, so the
pass repeats until nothing changes. Road geometry is preserved except for explicitly reported at-most-two-centimetre lane-width taper undershoot corrections.
Scenic rejects any negative width, including cubic interpolation overshoot in the export; larger defects fail.

usage: scenic_xodr.py IN.xodr OUT.xodr        or        sanitize(xodr_text) -> (text, report)
"""
import sys
import math
import xml.etree.ElementTree as ET


def repair_width_undershoot(root, tolerance=0.02):
    """Remove bounded centimetre-scale cubic taper undershoot rejected by Scenic.

    Check extrema, not just sampled widths: a cubic can become negative between
    positive endpoints. Refuse larger invalid geometry rather than silently
    changing the road layout. Each reported correction is a constant offset.
    """
    repairs = []
    for road in root.findall("road"):
        sections = road.findall("lanes/laneSection")
        for i, section in enumerate(sections):
            end = float(sections[i+1].get("s")) if i+1 < len(sections) else float(road.get("length"))
            length = end - float(section.get("s"))
            for lane in section.findall("./*/lane"):
                widths = lane.findall("width")
                maximum_width = 0.0
                for j, width in enumerate(widths):
                    end = float(widths[j+1].get("sOffset")) if j+1 < len(widths) else length
                    span = end - float(width.get("sOffset"))
                    a, b, c, d = [float(width.get(k, "0")) for k in "abcd"]
                    points = [0., span]
                    if d != 0:
                        disc = 4*c*c - 12*d*b
                        if disc >= 0:
                            points += [t for t in ((-2*c-math.sqrt(disc))/(6*d),
                                                  (-2*c+math.sqrt(disc))/(6*d)) if 0 < t < span]
                    elif c != 0 and 0 < -b/(2*c) < span:
                        points.append(-b/(2*c))
                    values = [((d*t+c)*t+b)*t+a for t in points]
                    minimum = min(values)
                    maximum_width = max(maximum_width, max(values))
                    if minimum >= 0: continue
                    if minimum < -tolerance:
                        raise ValueError(f"Road {road.get('id')} lane {lane.get('id')} has width {minimum:.6f} m; exceeds repair tolerance")
                    correction = -minimum + 1e-9
                    width.set("a", repr(a + correction))
                    repairs.append(dict(road=road.get("id"), lane=lane.get("id"),
                                        s_offset=float(width.get("sOffset")), correction_m=correction))
                if widths and maximum_width < 1e-7 and lane.get("type") in (
                        "driving", "onRamp", "offRamp", "entry", "exit", "connectingRamp"):
                    # Keep the lane ID/width for offset and linkage parsing, but
                    # do not ask Scenic to construct a driving polygon of zero area.
                    repairs.append(dict(road=road.get("id"), lane=lane.get("id"),
                                        excluded_zero_width=True, original_type=lane.get("type")))
                    lane.set("type", "none")
    return repairs


def sanitize(xodr_text):
    root = ET.fromstring(xodr_text)
    width_repairs = repair_width_undershoot(root)
    roads = {r.get("id"): r for r in root.findall("road")}
    junctions = {j.get("id"): j for j in root.findall("junction")}
    dropped = 0; dissolved = {}; rewritten = 0
    drivable = {"driving", "entry", "exit", "onRamp", "offRamp", "connectingRamp"}
    junction_lane_repairs = []
    # Explicit laneLinks must target a real drivable lane at the contact end.
    # Remove a connection if all its explicit links are invalid: an empty list
    # means implicit identity links to Scenic, not an unconnected road.
    for jid, junction in junctions.items():
        for conn in list(junction.findall("connection")):
            target = roads.get(conn.get("connectingRoad"))
            sections = target.findall("lanes/laneSection") if target is not None else []
            if not sections:
                continue
            section = sections[0 if conn.get("contactPoint") == "start" else -1]
            available = {l.get("id") for l in section.findall("./*/lane") if l.get("type") in drivable}
            explicit = conn.findall("laneLink")
            if not explicit:
                incoming = roads.get(conn.get("incomingRoad"))
                incoming_sections = incoming.findall("lanes/laneSection") if incoming is not None else []
                pred = incoming.find("link/predecessor") if incoming is not None else None
                at_start = pred is not None and pred.get("elementType") == "junction" and pred.get("elementId") == jid
                if incoming_sections:
                    incoming_section = incoming_sections[0 if at_start else -1]
                    incoming_ids = {l.get("id") for l in incoming_section.findall("./*/lane") if l.get("type") in drivable}
                    for lid in sorted(incoming_ids - available):
                        junction_lane_repairs.append(dict(junction=jid, connection=conn.get("id"),
                            incoming_road=conn.get("incomingRoad"), connecting_road=conn.get("connectingRoad"),
                            from_lane=lid, to_lane=lid, implicit_identity=True))
                    # Materialize only the valid part of the parser's identity
                    # fallback; never synthesize a cross-lane connection.
                    for lid in sorted(incoming_ids & available):
                        ET.SubElement(conn, "laneLink", {"from": lid, "to": lid})
                if not conn.findall("laneLink"):
                    junction.remove(conn)
                    dropped += 1
                continue
            for link in explicit:
                if link.get("to") not in available:
                    junction_lane_repairs.append(dict(junction=jid, connection=conn.get("id"),
                        incoming_road=conn.get("incomingRoad"), connecting_road=conn.get("connectingRoad"),
                        from_lane=link.get("from"), to_lane=link.get("to")))
                    conn.remove(link)
            if explicit and not conn.findall("laneLink"):
                junction.remove(conn)
                dropped += 1

    def is_plain(rid):
        return rid in roads and roads[rid].get("junction", "-1") == "-1"

    def successor_road(rid):
        s = roads[rid].find("link/successor")
        return s.get("elementId") if s is not None and s.get("elementType") == "road" and s.get("elementId") in roads else None

    def links_to_junction(rid, jid):
        link = roads[rid].find("link")
        return link is not None and any(el.get("elementType") == "junction" and el.get("elementId") == jid for el in link)

    def usable(conn, jid):
        inc, cr = conn.get("incomingRoad"), conn.get("connectingRoad")
        # both roads must exist, the incoming one must be a plain road that links to this junction (the parser reads which of
        # its ends touches the junction from that link) and the connecting one must lead somewhere (otherwise the parser has
        # no outgoing lane for the maneuver, and a junction without outgoing lanes cannot be ordered)
        if not (is_plain(inc) and links_to_junction(inc, jid) and cr in roads and successor_road(cr) is not None):
            return False
        connecting = roads[cr]
        successor = connecting.find("link/successor")
        target_sections = roads[successor_road(cr)].findall("lanes/laneSection")
        sections = connecting.findall("lanes/laneSection")
        if not target_sections or not sections:
            return False
        target_section = target_sections[0 if successor.get("contactPoint") == "start" else -1]
        available = {l.get("id") for l in target_section.findall("./*/lane") if l.get("type") in drivable}
        return any(l.get("type") in drivable and l.find("link/successor") is not None
                   and l.find("link/successor").get("id") in available
                   for l in sections[-1].findall("./*/lane"))

    changed = True
    while changed:
        changed = False
        for jid, j in list(junctions.items()):
            if any(usable(c, jid) for c in j.findall("connection")): continue
            members = [rid for rid, r in roads.items() if r.get("junction") == jid]
            for rid in members: roads[rid].set("junction", "-1")
            dissolved[jid] = members; root.remove(j); del junctions[jid]; changed = True
    for j in junctions.values():
        for conn in list(j.findall("connection")):
            if not usable(conn, j.get("id")): j.remove(conn); dropped += 1
    # roads linking to a dissolved junction: link to the member road that links back to them, else drop the link
    for rid, r in roads.items():
        link = r.find("link")
        if link is None: continue
        for tag in ("predecessor", "successor"):
            el = link.find(tag)
            if el is None or el.get("elementType") != "junction" or el.get("elementId") not in dissolved: continue
            target = None
            for m in dissolved[el.get("elementId")]:
                ml = roads[m].find("link")
                for mtag, contact in (("predecessor", "start"), ("successor", "end")):
                    mel = ml.find(mtag) if ml is not None else None
                    if mel is not None and mel.get("elementId") == rid: target = (m, contact)
            if target is None: link.remove(el); continue
            el.set("elementType", "road"); el.set("elementId", target[0]); el.set("contactPoint", target[1]); rewritten += 1
    # Some exported lane links name a lane which does not exist on the linked
    # road (e.g. road141 lane-2 -> road142 lane-4). CARLA terminates these paths;
    # Scenic indexes the missing lane and fails. Drop only the dangling link,
    # never guess a replacement or connect a taper to an unrelated lane.
    dangling = []
    drivable = {"driving", "entry", "exit", "onRamp", "offRamp", "connectingRamp"}
    for rid, road in roads.items():
        sections = road.findall("lanes/laneSection")
        if not sections: continue
        for tag, section in (("predecessor", sections[0]), ("successor", sections[-1])):
            link = road.find("link/" + tag)
            if link is None or link.get("elementType") != "road": continue
            target = roads.get(link.get("elementId"))
            target_sections = target.findall("lanes/laneSection") if target is not None else []
            if not target_sections: continue
            target_sec = target_sections[0 if link.get("contactPoint") == "start" else -1]
            available = {lane.get("id") for lane in target_sec.findall("./*/lane") if lane.get("type") in drivable}
            for lane in section.findall("./*/lane"):
                lane_link = lane.find("link/" + tag)
                if lane.get("type") in drivable and lane_link is not None and lane_link.get("id") not in available:
                    dangling.append(dict(road=rid, lane=lane.get("id"), end=tag,
                                         target_road=link.get("elementId"), target_lane=lane_link.get("id")))
                    lane.find("link").remove(lane_link)
    report = dict(dropped_connections=dropped, dissolved_junctions=[dict(id=k, roads=v) for k, v in dissolved.items()], rewritten_links=rewritten,
                  width_repairs=width_repairs, dropped_lane_links=dangling,
                  dropped_junction_lane_links=junction_lane_repairs)
    return ET.tostring(root, encoding="unicode"), report


if __name__ == "__main__":
    text, rep = sanitize(open(sys.argv[1]).read())
    open(sys.argv[2], "w").write(text)
    print(rep)
