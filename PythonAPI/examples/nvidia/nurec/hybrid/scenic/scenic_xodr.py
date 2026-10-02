"""OpenDRIVE from a NuRec artifact -> a copy Scenic's parser accepts.

The exporter writes junction connections whose incoming road is ``-1`` (none) or a road that itself belongs to a junction,
and junctions that end up with no usable connection at all. CARLA's OpenDRIVE loader ignores those; Scenic's
``xodr_parser`` only accepts plain roads as incoming roads and orders the roads of every junction around their centroid,
dividing by their number, which is zero for such a junction. This module drops the invalid connections and *dissolves*
every junction that keeps none: its connecting roads become plain roads and the neighbours' junction links are rewritten
to point at them. A dissolved junction can make a road plain that another junction uses as its incoming road, so the
pass repeats until nothing changes. The geometry is untouched: Scenic's lane centerlines still coincide with the proxy
world CARLA generates from the original file.

usage: scenic_xodr.py IN.xodr OUT.xodr        or        sanitize(xodr_text) -> (text, report)
"""
import sys
import xml.etree.ElementTree as ET


def sanitize(xodr_text):
    root = ET.fromstring(xodr_text)
    roads = {r.get("id"): r for r in root.findall("road")}
    junctions = {j.get("id"): j for j in root.findall("junction")}
    dropped = 0; dissolved = {}; rewritten = 0

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
        return is_plain(inc) and links_to_junction(inc, jid) and cr in roads and successor_road(cr) is not None

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
    report = dict(dropped_connections=dropped, dissolved_junctions=[dict(id=k, roads=v) for k, v in dissolved.items()], rewritten_links=rewritten)
    return ET.tostring(root, encoding="unicode"), report


if __name__ == "__main__":
    text, rep = sanitize(open(sys.argv[1]).read())
    open(sys.argv[2], "w").write(text)
    print(rep)
