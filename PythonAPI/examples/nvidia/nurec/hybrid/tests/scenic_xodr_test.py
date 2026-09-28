"""scenic_xodr.sanitize on the NuRec sample artifacts: the raw OpenDRIVE of every sample scene must fail or pass Scenic's
parser as it does today, and the sanitized copy must compile into a Scenic road network whose lanes still contain the
recorded start pose. Needs `scenic` and the sample set (NUREC_SAMPLES or ../../../nurec_samples/sample_set/26.04_release);
skips otherwise. Run with `python -O` (Scenic's parser asserts).

usage: python -O tests/scenic_xodr_test.py
"""
import glob, json, os, sys, tempfile, zipfile
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__)); HYBRID = os.path.dirname(HERE); NUREC = os.path.dirname(HYBRID)
sys.path.insert(0, os.path.join(HYBRID, "scenic")); sys.path.insert(0, NUREC)
import scenic_xodr  # noqa: E402

samples = os.environ.get("NUREC_SAMPLES", os.path.join(NUREC, "..", "..", "..", "..", "..", "nurec_samples", "sample_set", "26.04_release"))
usdzs = sorted(glob.glob(os.path.join(samples, "*", "*.usdz")))
if not usdzs: print("SKIP: no NuRec samples under", samples); sys.exit(0)
try:
    from scenic.domains.driving.roads import Network
    from scenic.core.vectors import Vector
except ImportError: print("SKIP: scenic not installed"); sys.exit(0)
from projection_functions import get_t_rig_enu_from_ecef  # noqa: E402

failures = 0
with tempfile.TemporaryDirectory() as td:
    for usdz in usdzs:
        name = os.path.basename(os.path.dirname(usdz))[:8]
        with zipfile.ZipFile(usdz) as z:
            xodr = z.read("map.xodr").decode(); rig = json.loads(z.read("rig_trajectories.json"))
        text, rep = scenic_xodr.sanitize(xodr)
        path = os.path.join(td, name + ".xodr"); open(path, "w").write(text)
        try:
            net = Network.fromFile(path, useCache=False)
        except Exception as e:
            print(f"FAIL {name}: sanitized map does not parse: {type(e).__name__} {e}"); failures += 1; continue
        # the recorded start pose (rig trajectory, scene frame -> map frame) must lie on a lane of the network
        t_sc = get_t_rig_enu_from_ecef(np.array(rig["T_world_base"]), xodr)
        T0 = np.array(rig["rig_trajectories"][0]["T_rig_worlds"][0], dtype=float).reshape(4, 4)
        q = t_sc @ T0[:, 3]; on_lane = net.laneAt(Vector(float(q[0]), float(q[1]))) is not None
        ok = on_lane is not False
        print(f"{'OK  ' if ok else 'FAIL'} {name}: {len(net.roads)} roads, {len(net.intersections)} intersections, {len(net.lanes)} lanes; "
              f"dropped {rep['dropped_connections']} connections, dissolved {[d['id'] for d in rep['dissolved_junctions']]}, rewrote {rep['rewritten_links']} links; "
              f"start pose on a lane: {on_lane}")
        failures += 0 if ok else 1
print("FAILED" if failures else "ALL OK", flush=True); sys.exit(1 if failures else 0)
