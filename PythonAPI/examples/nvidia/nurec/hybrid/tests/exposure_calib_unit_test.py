#!/usr/bin/env python3
"""Unit test of exposure_calib.py without a server: synthetic CARLA-layer / neural pairs with a known EV offset.

A textured road plane (tag Roads, 4..80 m) is rendered on both sides through the sRGB 8-bit encode; the neural side
gets cast shadows (-2 EV patches) and real markings (+1.6 EV patches) the bare proxy does not have, a parked-car region
whose neural distance disagrees with the proxy distance, a low-opacity region, a RoadLines band and clipped pixels.
Checks: the mask keeps only what it should, the per-frame EV is within 0.05 EV of the truth with the +-1 EV trim dropping
exactly the shadow / marking pixels, and `solve_ev` converges through a compressive (tonemapper-like) response to within
the tolerance.  usage: exposure_calib_unit_test.py [-v]
"""
import os
import sys
import unittest

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import exposure_calib as ec  # noqa: E402

H, W = 240, 320
EV_TRUE = 3.4                    # sensor EV at which the proxy road matches the neural road


def linear_to_srgb8(lin):
    x = np.clip(lin, 0.0, 1.0)
    s = np.where(x <= 0.0031308, 12.92 * x, 1.055 * x ** (1 / 2.4) - 0.055)
    return np.clip(np.round(s * 255.0), 0, 255).astype(np.uint8)


def tonemap(lin):
    """Compressive, monotonic response (Reinhard-like) so that the 8-bit output is not linear in 2^EV."""
    return lin / (1.0 + 0.6 * lin)


class Scene:
    """Synthetic road pair. Regions are horizontal bands / patches so the expected mask is known exactly."""

    def __init__(self, seed=0):
        rng = np.random.default_rng(seed)
        v = np.arange(H)[:, None] * np.ones((1, W))
        self.dist = 3.0 + 90.0 * (H - v) / H                              # 93 m at the top row .. 3 m at the bottom
        self.tag = np.full((H, W), ec.ROADS, np.uint8)
        self.tag[:, 150:160] = ec.ROADLINES                                # a painted line down the road
        albedo = 0.08 * np.exp(rng.normal(0.0, 0.15, (H, W)))              # asphalt with texture
        self.base = albedo * 0.6                                           # linear road radiance (neutral colour)
        self.neural_gain = np.ones((H, W))
        self.shadow = np.zeros((H, W), bool); self.shadow[60:90, 20:140] = True; self.shadow[150:170, 200:300] = True
        self.neural_gain[self.shadow] = 0.25                               # -2 EV: a tree / bridge shadow the proxy lacks
        self.marking = np.zeros((H, W), bool); self.marking[100:106, 30:130] = True; self.marking[180:184, 40:120] = True
        self.neural_gain[self.marking] = 3.0                               # +1.6 EV: a real marking under a proxy road pixel
        self.line_gain = np.ones((H, W)); self.line_gain[self.tag == ec.ROADLINES] = 3.0   # the proxy paints its line too
        self.dist_e = self.dist.copy()
        self.car = np.zeros((H, W), bool); self.car[40:70, 220:290] = True
        self.dist_e[self.car] = self.dist[self.car] * 0.5                  # a parked car in the neural scene, in front of the proxy road
        self.opa = np.ones((H, W), np.float32); self.lowopa = np.zeros((H, W), bool); self.lowopa[20:30, :] = True; self.opa[self.lowopa] = 0.4
        self.clip = np.zeros((H, W), bool); self.clip[200:210, 250:300] = True

    def neural(self):
        lin = tonemap(self.base * self.neural_gain * self.line_gain * 2.0 ** EV_TRUE)
        lin = np.where(self.clip, 5.0, lin)
        return np.repeat(linear_to_srgb8(lin)[..., None], 3, axis=-1)

    def carla(self, ev):
        lin = tonemap(self.base * self.line_gain * 2.0 ** ev)
        return np.repeat(linear_to_srgb8(lin)[..., None], 3, axis=-1)

    def stats(self, ev):
        return ec.frame_stats(self.carla(ev), self.tag, self.dist, self.neural(), self.dist_e, self.opa)


class MaskTest(unittest.TestCase):
    def setUp(self): self.sc = Scene()

    def test_mask_rejects_what_it_should(self):
        _, road = self.sc.stats(EV_TRUE)
        sc = self.sc
        self.assertFalse(road[sc.car].any(), "distance-mismatch pixels (parked car) must be rejected")
        self.assertFalse(road[sc.lowopa].any(), "low-opacity pixels must be rejected")
        self.assertFalse(road[sc.clip].any(), "clipped neural pixels must be rejected")
        self.assertFalse(road[sc.tag == ec.ROADLINES].any(), "RoadLines pixels are not road")
        self.assertFalse(road[sc.dist <= 4.0].any() or road[sc.dist >= 80.0].any(), "outside 4..80 m must be rejected")
        keep = (sc.tag == ec.ROADS) & ~sc.car & ~sc.lowopa & ~sc.clip & (sc.dist > 4.0) & (sc.dist < 80.0)
        self.assertGreater(road[keep].mean(), 0.995, "plain road pixels must be kept")
        # cast shadows and markings are geometrically road on both sides: they stay in the mask (bar the few marking pixels
        # that clip) and are handled by the trim
        self.assertGreater(road[sc.shadow & keep].mean(), 0.99); self.assertGreater(road[sc.marking & keep].mean(), 0.9)

    def test_trimmed_median_within_tolerance(self):
        for ev in (EV_TRUE, EV_TRUE + 0.7, EV_TRUE - 1.2):
            st, _ = self.sc.stats(ev)
            expected = np.log2(tonemap(0.048 * 2 ** ev) / tonemap(0.048 * 2 ** EV_TRUE))    # response at the median albedo
            self.assertGreater(st["road_px"], ec.MIN_ROAD_PX)
            self.assertLess(abs(st["road_ev"] - expected), 0.05, f"EV {ev}: road_ev {st['road_ev']:+.3f} vs {expected:+.3f}")
            self.assertGreater(st["road_trim_frac"], 0.05, "shadow and marking pixels must be trimmed")
        st, road = self.sc.stats(EV_TRUE)
        self.assertLess(abs(st["road_ev"]), 0.02)
        outl = ((self.sc.shadow | self.sc.marking) & road).sum() / road.sum()      # what the +-1 EV trim must drop, nothing else
        self.assertLess(abs(st["road_trim_frac"] - outl), 0.02, f"trimmed {st['road_trim_frac']:.3f} vs outliers {outl:.3f}")
        # markings cross-check: the proxy line is as bright as the real one here, so it reads ~0 EV too
        self.assertGreater(st["lines_px"], 300); self.assertLess(abs(st["lines_ev"]), 0.1)

    def test_too_few_pixels_is_nan(self):
        sc = self.sc; tag = sc.tag.copy(); tag[:, :] = 0; tag[100:110, 100:150] = ec.ROADS
        st, _ = ec.frame_stats(sc.carla(EV_TRUE), tag, sc.dist, sc.neural(), sc.dist_e, sc.opa)
        self.assertLess(st["road_px"], ec.MIN_ROAD_PX); self.assertTrue(np.isnan(st["road_ev"]))


class SolveTest(unittest.TestCase):
    def test_solve_converges(self):
        scenes = [Scene(seed=s) for s in range(3)]
        calls = []

        def measure(ev):
            calls.append(ev)
            return [dict(scenes[i].stats(ev)[0], k=i) for i in range(len(scenes))]

        out, calib = ec.solve_ev(measure, ec0=5.0, tol=0.05, log=lambda *a: None)
        self.assertLess(abs(out - EV_TRUE), 0.05, f"solved {out:.3f} vs {EV_TRUE}")
        self.assertTrue(calib["converged"]); self.assertLess(abs(calib["residual_ev"]), 0.05); self.assertLessEqual(len(calls), 5)
        self.assertEqual(calib["iterations"], len(calls)); self.assertEqual(len(calib["per_frame"]), 3)

    def test_fixed_only_measures(self):
        sc = Scene()
        out, calib = ec.solve_ev(lambda ev: [sc.stats(ev)[0]], ec0=5.0, fixed=True, log=lambda *a: None)
        self.assertEqual(out, 5.0); self.assertEqual(calib["mode"], "fixed"); self.assertEqual(calib["iterations"], 1)
        self.assertGreater(calib["residual_ev"], 0.5)

    def test_no_road_exits(self):
        sc = Scene(); tag = np.zeros((H, W), np.uint8)
        with self.assertRaises(SystemExit) as cm:
            ec.solve_ev(lambda ev: [ec.frame_stats(sc.carla(ev), tag, sc.dist, sc.neural(), sc.dist_e, sc.opa)[0]], log=lambda *a: None)
        self.assertIn("road pixels", str(cm.exception))
        with self.assertRaises(SystemExit): ec.solve_ev(lambda ev: [], log=lambda *a: None)


if __name__ == "__main__":
    unittest.main()
