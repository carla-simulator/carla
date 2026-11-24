#!/usr/bin/env python3
"""Turn a scenario spec into a valid .scenic file, then prove it constructs.

The natural-language -> spec step is the agent's job; this script owns the part
that must be exact: the boilerplate, the placement idioms, and the validation
loop. Every emitted idiom is lifted from a scenario that runs on this build, so a
generated file fails for reasons in the spec, never in the syntax.

  python3 scaffold_scenic.py --spec s.json --out my.scenic     generate
  python3 scaffold_scenic.py --validate my.scenic              compile + sample
  python3 scaffold_scenic.py --example                         print a spec

Spec keys (all optional but `map`):
  name, description   header text
  map                 e.g. "Town05" - must have a matching .xodr in assets
  model               "srunner" (default) or "scenic"
  xodr                explicit .xodr path, overriding the assets lookup
  placement           "lane" | "intersection"  (default "lane")
  arms                4 | 3        intersection arms                (intersection)
  signalized          true | false | null=don't care                (intersection)
  ego                 {blueprint, speed, maneuver: straight|left|right, brake_distance}
  actors              [{name, type, blueprint, relation, distance, speed, maneuver,
                        cut_in, cut_in_delay}]
                      relation: ahead | behind | right_lane | left_lane | conflicting
                      right_lane/left_lane cut in front of the ego unless cut_in is false
  speed               a number, or [lo, hi] for a sampled Range (m/s)
  requires            ["(distance to intersection) > 50", ...]
  terminate           e.g. "(distance to ego_spawn) > 70"
  timestep            simulation timestep, default 0.1
"""
from __future__ import annotations
import argparse, json, os, sys, textwrap, warnings
from pathlib import Path

MODELS = {"srunner": "srunner.scenic.models.model",
          "scenic": "scenic.simulators.carla.model"}

EXAMPLE = {
    "name": "CutInFromRight",
    "description": "A car in the right lane cuts in front of the ego on a straight road.",
    "map": "Town05",
    "model": "srunner",
    "placement": "lane",
    "ego": {"blueprint": "vehicle.lincoln.mkz", "speed": [8, 11]},
    "actors": [{"name": "cutin_car", "type": "Car", "blueprint": "vehicle.dodge.charger",
                "relation": "right_lane", "distance": [8, 20], "speed": [6, 9],
                "cut_in_delay": [0.5, 3.0]}],
    "requires": ["(distance to intersection) > 40"],
    "terminate": "(distance to ego_spawn) > 70",
}


def num(v) -> str:
    """A spec number, or a [lo, hi] pair as a Scenic Range sampled per scene."""
    if isinstance(v, (list, tuple)):
        return "Range(%s, %s)" % (v[0], v[1])
    return str(v)


def cuts_in(act: dict) -> bool:
    """right_lane/left_lane actors change into the ego's lane unless told not to;
    a car that only sits in the neighbouring lane is not a cut-in."""
    return act.get("relation") in ("right_lane", "left_lane") and act.get("cut_in", True)


def find_xodr(spec: dict) -> str:
    """Resolve the OpenDrive file for `map`. A scenario's `param map` is what
    Scenic builds the road network from; `carla_map` only tells CARLA what to
    load. Both must name the same town or the scene is placed on one map and
    simulated on another."""
    if spec.get("xodr"):
        return spec["xodr"]
    town = spec["map"]
    # Map packages ship as TownXX_Opt but the OpenDrive assets are named without
    # the suffix, so `carla_map` and `map` legitimately differ here. Try both.
    names = [town] + ([town[:-4]] if town.endswith("_Opt") else [town + "_Opt"])
    roots = []
    sr = os.environ.get("SCENARIO_RUNNER_ROOT")
    if sr:
        roots.append(Path(sr) / "srunner/scenic/assets")
    se = os.environ.get("SCENIC_EXAMPLES")
    if se:
        roots.append(Path(se).resolve().parents[1] / "assets/maps/CARLA")
    sroot = os.environ.get("SCENIC_ROOT")
    if sroot:
        roots.append(Path(sroot) / "assets/maps/CARLA")
    for r in roots:
        for nm in names:
            p = r / f"{nm}.xodr"
            if p.is_file():
                return str(p)
    raise SystemExit(f"no .xodr for {town} (tried {', '.join(names)}). Looked in:\n  " +
                     "\n  ".join(str(r) for r in roots) +
                     "\nPass \"xodr\" in the spec to point at one explicitly.")


def emit(spec: dict) -> str:
    model = MODELS[spec.get("model", "srunner")]
    ego = spec.get("ego") or {}
    ego_bp = ego.get("blueprint", "vehicle.lincoln.mkz")
    ego_speed = ego.get("speed", 10)
    placement = spec.get("placement", "lane")
    actors = spec.get("actors") or []
    crossing = any(a.get("relation") == "crossing" for a in actors)
    L = []
    A = L.append

    A('""" %s' % (spec.get("name") or "Generated scenario"))
    for line in textwrap.wrap(spec.get("description", ""), 78):
        A(line)
    A('"""')
    A("")
    A("param map = localPath(%r)" % find_xodr(spec))
    A("param carla_map = %r" % spec["map"])
    A("param timestep = %s" % spec.get("timestep", 0.1))
    A("model %s" % model)
    A("")
    A("EGO_MODEL = %r" % ego_bp)
    A("EGO_SPEED = %s" % num(ego_speed))
    any_cut_in = placement != "intersection" and any(cuts_in(a) for a in actors)
    if any_cut_in:
        A("EGO_BRAKE_DIST = %s" % ego.get("brake_distance", 9))
    for i, act in enumerate(actors):
        A("ACTOR%d_SPEED = %s" % (i, num(act.get("speed", 8))))
        if cuts_in(act):
            A("ACTOR%d_CUT_IN_DELAY = %s" % (i, num(act.get("cut_in_delay", [0.5, 3.0]))))
    A("")

    # --- behaviors -----------------------------------------------------------
    if placement == "intersection":
        A("behavior EgoBehavior(trajectory):")
        A("    do FollowTrajectoryBehavior(target_speed=EGO_SPEED, trajectory=trajectory)")
    elif any_cut_in:
        # Brake for whatever ends up in the ego's lane ahead, or the cut-in is a
        # rear-end collision rather than a situation.
        A("behavior EgoBehavior():")
        A("    try:")
        A("        do FollowLaneBehavior(target_speed=EGO_SPEED)")
        A("    interrupt when withinDistanceToObjsInLane(self, EGO_BRAKE_DIST):")
        A("        take SetBrakeAction(0.8)")
    else:
        A("behavior EgoBehavior():")
        A("    do FollowLaneBehavior(target_speed=EGO_SPEED)")
    A("")
    for i, act in enumerate(actors):
        rel = act.get("relation", "ahead")
        A("behavior Actor%dBehavior(%s):" % (i, "trajectory" if rel == "conflicting" else ""))
        if rel == "conflicting":
            A("    do FollowTrajectoryBehavior(target_speed=ACTOR%d_SPEED, trajectory=trajectory)" % i)
        elif rel == "crossing":
            # CrossingBehavior waits for the ego to close to `threshold` metres,
            # then walks across; a plain FollowLaneBehavior does not apply to walkers.
            A("    do CrossingBehavior(ego, ACTOR%d_SPEED, %s)" % (i, act.get("threshold", 10)))
        elif cuts_in(act):
            # A car placed in the right lane cuts in by moving to *its* left, the
            # ego's lane, and vice versa. Read the neighbour from the section the
            # car is on now, not the one it spawned on: it has driven since.
            toward = "Left" if act["relation"] == "right_lane" else "Right"
            A("    do FollowLaneBehavior(target_speed=ACTOR%d_SPEED) for ACTOR%d_CUT_IN_DELAY seconds" % (i, i))
            A("    do LaneChangeBehavior(laneSectionToSwitch=self.laneSection._laneTo%s, target_speed=ACTOR%d_SPEED)"
              % (toward, i))
            A("    do FollowLaneBehavior(target_speed=ACTOR%d_SPEED)" % i)
        else:
            A("    do FollowLaneBehavior(target_speed=ACTOR%d_SPEED)" % i)
        A("")

    # --- geometry ------------------------------------------------------------
    if placement == "intersection":
        arms = spec.get("arms", 4)
        sig = spec.get("signalized", None)
        pred = ["i.is%dWay" % arms]
        if sig is True:
            pred.append("i.isSignalized")
        elif sig is False:
            pred.append("not i.isSignalized")
        A("# A filter matching nothing raises 'discrete distribution over empty")
        A("# domain' at compile time. list_scenic.py --check-maps shows the counts.")
        A("candidates = filter(lambda i: %s, network.intersections)" % " and ".join(pred))
        A("assert len(candidates) > 0, 'no matching intersection on %s'" % spec["map"])
        A("intersec = Uniform(*candidates)")
        A("startLane = Uniform(*intersec.incomingLanes)")
        man = (ego.get("maneuver") or "straight").upper()
        # No assert here: startLane comes from Uniform(), so anything derived from
        # it is a random value and `len(...) > 0` raises RandomControlFlowError.
        # An arm with no such maneuver is rejected at sample time instead.
        A("ego_maneuvers = filter(lambda m: m.type == ManeuverType.%s, startLane.maneuvers)" % man)
        A("ego_maneuver = Uniform(*ego_maneuvers)")
        A("ego_trajectory = [ego_maneuver.startLane, ego_maneuver.connectingLane, ego_maneuver.endLane]")
        A("ego_spawn = new OrientedPoint in ego_maneuver.startLane.centerline")
        A("")
        A("ego = new Car at ego_spawn,")
        A("    with blueprint EGO_MODEL,")
        A("    with behavior EgoBehavior(ego_trajectory)")
    else:
        needs_adjacent = any(a.get("relation") in ("right_lane", "left_lane") for a in actors)
        if needs_adjacent:
            side = "right" if any(a.get("relation") == "right_lane" for a in actors) else "left"
            A("# Only lane *sections* know their neighbours, so select on sections.")
            A("# The neighbour must run the same way: _laneToLeft can be the oncoming")
            A("# lane, and a car placed there drives at the ego instead of beside it.")
            A("laneSecs = []")
            A("for lane in network.lanes:")
            A("    for sec in lane.sections:")
            A("        nb = sec._laneTo%s" % side.capitalize())
            A("        if nb != None and nb.isForward == sec.isForward:")
            A("            laneSecs.append(sec)")
            A("assert len(laneSecs) > 0, 'no lane section with a same-direction %s neighbour on %s'" % (side, spec["map"]))
            A("initLaneSec = Uniform(*laneSecs)")
            A("adjacentLane = initLaneSec._laneTo%s" % side.capitalize())
            A("ego_spawn = new OrientedPoint on initLaneSec.centerline")
        else:
            A("lane = Uniform(*network.lanes)")
            A("ego_spawn = new OrientedPoint on lane.centerline")
        A("")
        if crossing:
            # Mirrors the shipped crossing scenarios: the anchor is the crossing
            # point and the ego starts back up the lane so it drives into it.
            A("ego = new Car following roadDirection from ego_spawn for Range(-40, -35),")
        else:
            A("ego = new Car at ego_spawn,")
        A("    with blueprint EGO_MODEL,")
        A("    with behavior EgoBehavior()")
    A("")

    # --- other actors --------------------------------------------------------
    post = []   # requirements an actor's placement implies, emitted with the spec's
    for i, act in enumerate(actors):
        nm = act.get("name") or "actor%d" % i
        typ = act.get("type", "Car")
        rel = act.get("relation", "ahead")
        lo, hi = (act.get("distance") or [10, 30])[:2]
        if rel == "conflicting":
            A("adv%d_maneuvers = filter(lambda m: m.type == ManeuverType.%s, ego_maneuver.conflictingManeuvers)"
              % (i, (act.get("maneuver") or "straight").upper()))
            A("adv%d_maneuver = Uniform(*adv%d_maneuvers)" % (i, i))
            A("adv%d_trajectory = [adv%d_maneuver.startLane, adv%d_maneuver.connectingLane, adv%d_maneuver.endLane]"
              % (i, i, i, i))
            A("%s = new %s at (new OrientedPoint in adv%d_maneuver.startLane.centerline)," % (nm, typ, i))
            A("    with behavior Actor%dBehavior(adv%d_trajectory)" % (i, i))
        elif rel == "crossing":
            A("%s = new %s right of ego_spawn by %s," % (nm, typ, act.get("offset", 3)))
            A("    with heading 90 deg relative to ego_spawn.heading,")
            A("    with behavior Actor%dBehavior()" % i)
        elif rel in ("right_lane", "left_lane"):
            A("%s = new %s on adjacentLane.centerline," % (nm, typ))
            A("    with behavior Actor%dBehavior()" % i)
            if cuts_in(act):
                # Ahead of the ego by `distance`, with road left to change lanes
                # before a junction. Without these the car is as likely to spawn
                # behind the ego, and the cut-in never happens in front of it.
                # Keep distance[0] above a car length (~5 m): closer, the two
                # spawn side by side and the lane change wedges into the ego.
                post.append("(relative position of %s from ego).y > %s" % (nm, lo))
                post.append("(distance from ego to %s) < %s" % (nm, hi))
                post.append("(distance from %s to intersection) > 30" % nm)
        elif rel == "behind":
            A("%s = new %s following roadDirection from ego for -Range(%s, %s)," % (nm, typ, lo, hi))
            A("    with behavior Actor%dBehavior()" % i)
        else:  # ahead
            A("%s = new %s following roadDirection from ego for Range(%s, %s)," % (nm, typ, lo, hi))
            A("    with behavior Actor%dBehavior()" % i)
        if act.get("blueprint"):
            L[-1] = L[-1] + ","
            A("    with blueprint %r" % act["blueprint"])
        # Pedestrians and props default to a containment region they are not being
        # placed in (walkable area / road), and containment is checked every sample,
        # so without this every sample is rejected and generation never converges.
        if typ != "Car" and typ != "Truck":
            L[-1] = L[-1] + ","
            A("    with regionContainedIn None")
        A("")

    for r in post + (spec.get("requires") or []):
        A("require %s" % r)
    A("terminate when %s" % (spec.get("terminate") or "(distance to ego_spawn) > 70"))
    return "\n".join(L) + "\n"


def validate(path: str, iterations: int = 2000) -> int:
    """Compile, then sample. Compile catches syntax and empty categories; sampling
    catches requirements no road geometry on this map can satisfy."""
    warnings.filterwarnings("ignore")
    import scenic
    try:
        sc = scenic.scenarioFromFile(path, mode2D=True)
    except Exception as e:
        print("  COMPILE-FAIL %s: %s" % (type(e).__name__, str(e).replace("\n", " ")[:300]))
        return 1
    print("  compiled: map=%s" % sc.params.get("carla_map", "?"))
    try:
        scene, iters = sc.generate(maxIterations=iterations)
    except Exception as e:
        print("  SAMPLE-FAIL %s: %s" % (type(e).__name__, str(e).replace("\n", " ")[:300]))
        print("  the requirements are unsatisfiable on this map — loosen one, or change map")
        return 1
    bps = sorted({getattr(o, "blueprint", None) for o in scene.objects} - {None})
    print("  PASS sampled in %d iterations: %d objects" % (iters, len(scene.objects)))
    for b in bps:
        print("    blueprint", b)
    print("  now simulate it with the run-scenic-scenario skill")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--spec")
    ap.add_argument("--out")
    ap.add_argument("--validate")
    ap.add_argument("--iterations", type=int, default=2000)
    ap.add_argument("--example", action="store_true")
    a = ap.parse_args()

    if a.example:
        print(json.dumps(EXAMPLE, indent=2))
        return 0
    if a.validate:
        return validate(a.validate, a.iterations)
    if not a.spec:
        ap.error("need --spec, --validate or --example")
    spec = json.load(open(a.spec))
    if "map" not in spec:
        raise SystemExit('spec needs a "map" key')
    text = emit(spec)
    if a.out:
        Path(a.out).write_text(text)
        print("wrote %s" % a.out)
        return validate(a.out, a.iterations)
    sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
