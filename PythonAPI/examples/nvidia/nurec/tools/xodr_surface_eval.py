#!/usr/bin/env python3
"""Ground-truth OpenDRIVE surface evaluator (lines+arcs, elevation, lateral shapes).

For a query xy point and a road id, finds the road-frame (s,t) and returns the
surface z with and without the lateralProfile/shape contribution. Used to check
whether crossing roads' surfaces reconcile once shapes are honored, and to
compare against CARLA's generated mesh.
"""
import math
import sys
import xml.etree.ElementTree as ET


class Road:
    def __init__(self, el):
        self.id = el.get('id')
        self.length = float(el.get('length'))
        self.geoms = []
        for g in el.findall('./planView/geometry'):
            rec = {
                's': float(g.get('s')), 'x': float(g.get('x')),
                'y': float(g.get('y')), 'hdg': float(g.get('hdg')),
                'length': float(g.get('length'))}
            arc = g.find('arc')
            rec['curv'] = float(arc.get('curvature')) if arc is not None else 0.0
            self.geoms.append(rec)
        self.elev = [(float(e.get('s')), float(e.get('a')), float(e.get('b')),
                      float(e.get('c')), float(e.get('d')))
                     for e in el.findall('./elevationProfile/elevation')]
        self.shapes = {}  # s_station -> list of (t, a, b, c, d) sorted by t
        for sh in el.findall('./lateralProfile/shape'):
            s = float(sh.get('s'))
            self.shapes.setdefault(s, []).append(
                (float(sh.get('t')), float(sh.get('a')), float(sh.get('b')),
                 float(sh.get('c')), float(sh.get('d'))))
        for v in self.shapes.values():
            v.sort()

    def ref_point(self, s):
        g = None
        for cand in self.geoms:
            if cand['s'] <= s + 1e-9:
                g = cand
        ds = s - g['s']
        x, y, hdg, k = g['x'], g['y'], g['hdg'], g['curv']
        if abs(k) < 1e-12:
            return x + ds * math.cos(hdg), y + ds * math.sin(hdg), hdg
        # arc
        hdg2 = hdg + k * ds
        x2 = x + (math.sin(hdg2) - math.sin(hdg)) / k
        y2 = y - (math.cos(hdg2) - math.cos(hdg)) / k
        return x2, y2, hdg2

    def elevation(self, s):
        rec = None
        for e in self.elev:
            if e[0] <= s + 1e-9:
                rec = e
        if rec is None:
            return 0.0
        ds = s - rec[0]
        return rec[1] + rec[2] * ds + rec[3] * ds ** 2 + rec[4] * ds ** 3

    def shape_station_eval(self, recs, t):
        """Piecewise eval at one s-station: pick record with greatest t_rec <= t
        (clamp to first)."""
        rec = recs[0]
        for cand in recs:
            if cand[0] <= t + 1e-9:
                rec = cand
        dt = t - rec[0]
        return rec[1] + rec[2] * dt + rec[3] * dt ** 2 + rec[4] * dt ** 3

    def shape_z(self, s, t):
        if not self.shapes:
            return 0.0
        stations = sorted(self.shapes)
        lo = stations[0]
        for st in stations:
            if st <= s + 1e-9:
                lo = st
        hi = None
        for st in stations:
            if st > s + 1e-9:
                hi = st
                break
        z_lo = self.shape_station_eval(self.shapes[lo], t)
        if hi is None or hi == lo:
            return z_lo
        z_hi = self.shape_station_eval(self.shapes[hi], t)
        w = (s - lo) / (hi - lo)
        w = max(0.0, min(1.0, w))
        return z_lo * (1 - w) + z_hi * w

    def project(self, px, py, step0=1.0):
        """Find (s, t) of xy point via dense sampling + refinement."""
        best = (None, 1e18)
        s = 0.0
        while s <= self.length:
            x, y, _ = self.ref_point(s)
            d2 = (x - px) ** 2 + (y - py) ** 2
            if d2 < best[1]:
                best = (s, d2)
            s += step0
        s0 = best[0]
        for step in (0.1, 0.01, 0.001):
            lo = max(0.0, s0 - step * 10)
            hi = min(self.length, s0 + step * 10)
            s = lo
            best = (s0, best[1])
            while s <= hi:
                x, y, _ = self.ref_point(s)
                d2 = (x - px) ** 2 + (y - py) ** 2
                if d2 < best[1]:
                    best = (s, d2)
                s += step
            s0 = best[0]
        x, y, hdg = self.ref_point(s0)
        # signed t: positive to the left of heading
        t = -(px - x) * math.sin(hdg) + (py - y) * math.cos(hdg)
        return s0, t


def load(path):
    tree = ET.parse(path)
    return {r.get('id'): Road(r) for r in tree.getroot().findall('road')}


if __name__ == '__main__':
    xodr = sys.argv[1]
    px, py = float(sys.argv[2]), float(sys.argv[3])
    road_ids = sys.argv[4:]
    roads = load(xodr)
    print("query point (%.2f, %.2f)" % (px, py))
    for rid in road_ids:
        r = roads[rid]
        s, t = r.project(px, py)
        ze = r.elevation(s)
        zs = r.shape_z(s, t)
        print("road %-4s s=%8.3f t=%7.3f  elev=%8.3f  shape_dz=%7.3f  "
              "surface=%8.3f  (no-shape surface=%8.3f)" % (
                  rid, s, t, ze, zs, ze + zs, ze))
