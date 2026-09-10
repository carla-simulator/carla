import { test } from "node:test";
import assert from "node:assert/strict";
import { SCENE_IDS, buildScene, centerlineFrom, listScenes, poseOnPath, rangeAt } from "../src/scenes";

test("a curvature profile integrates into the expected geometry", () => {
  const straight = centerlineFrom([{ length_m: 100, curvature: 0 }]);
  assert.equal(straight[0].x, 0);
  assert.equal(straight[straight.length - 1].x, 100);
  assert.equal(straight[straight.length - 1].y, 0);

  // A quarter of a 100 m-radius circle ends one radius over and one radius up.
  const quarter = centerlineFrom([{ length_m: (Math.PI / 2) * 100, curvature: 0.01 }]);
  const end = quarter[quarter.length - 1];
  assert.ok(Math.abs(end.x - 100) < 2, `x was ${end.x}`);
  assert.ok(Math.abs(end.y - 100) < 2, `y was ${end.y}`);
});

test("a lateral offset is applied to the left of the direction of travel", () => {
  const points = centerlineFrom([{ length_m: 50, curvature: 0 }]);
  const cum = points.map((_, i) => i * 2);
  const left = poseOnPath(points, cum, 20, 3);
  assert.equal(Math.round(left.x), 20);
  assert.equal(Math.round(left.y), 3);
  const right = poseOnPath(points, cum, 20, -3);
  assert.equal(Math.round(right.y), -3);
  // Past the end of the path the pose clamps rather than extrapolating.
  assert.equal(Math.round(poseOnPath(points, cum, 999).x), 50);
});

test("every demo scene is generated deterministically", () => {
  for (const id of SCENE_IDS) {
    assert.deepEqual(buildScene(id), buildScene(id), `${id} is not reproducible`);
  }
});

test("an unknown scene id is a miss, not a throw", () => {
  assert.equal(buildScene("does-not-exist"), null);
});

test("tracks are sampled at the scene rate for the full duration", () => {
  for (const id of SCENE_IDS) {
    const scene = buildScene(id)!;
    const frames = scene.duration_s * scene.hz + 1;
    assert.equal(scene.ego.track.length, frames, `${id} ego`);
    assert.equal(scene.ego.track[0].t, 0);
    assert.equal(scene.ego.track[frames - 1].t, scene.duration_s);
    for (const actor of scene.actors) assert.equal(actor.track.length, frames, `${id} ${actor.actor_id}`);
  }
});

test("the summary list matches the scenes it summarises", () => {
  const summaries = listScenes();
  assert.deepEqual(summaries.map((s) => s.scene_id), SCENE_IDS);
  for (const summary of summaries) {
    const scene = buildScene(summary.scene_id)!;
    assert.equal(summary.actors, scene.actors.length);
    assert.equal(summary.events, scene.events.length);
    assert.equal(summary.lidar_range_m, scene.sensor.lidar_range_m);
    assert.equal(summary.duration_s, scene.duration_s);
  }
});

test("degraded-visibility scenes hold the ODD cells the coverage matrix reports as gaps", () => {
  const odd = listScenes().map((s) => `${s.odd.visibility_class}/${s.odd.lighting_class}`);
  assert.ok(odd.includes("fog/night"), "no fog at night");
  assert.ok(odd.includes("heavy_rain/daylight"), "no heavy rain");
  // Every scene carries the three classes the scenario library groups on.
  for (const scene of listScenes()) {
    assert.ok(scene.odd.visibility_class && scene.odd.lighting_class && scene.odd.route_class);
  }
});

test("fog and rain cut how early an actor can be detected", () => {
  const clear = buildScene("thimphu_junction")!;
  const fog = buildScene("night_fog_pass")!;
  assert.ok(fog.sensor.lidar_range_m < clear.sensor.lidar_range_m);

  // The junction pedestrian is on sensor a long way out; the fog pedestrian is
  // not acquired until the ego is nearly on top of them. That contrast is the
  // whole reason these two scenes ship together.
  const firstSeen = (scene: ReturnType<typeof buildScene>, actorId: string) => {
    const s = scene!;
    const actor = s.actors.find((a) => a.actor_id === actorId)!;
    for (let f = 0; f < s.ego.track.length; f += 1) {
      if (rangeAt(s, actor, f) <= s.sensor.lidar_range_m) return { t: f / s.hz, range: rangeAt(s, actor, f) };
    }
    return null;
  };
  const junction = firstSeen(clear, "ped-05")!;
  const night = firstSeen(fog, "ped-01")!;
  assert.ok(junction.range > 70, `junction pedestrian seen at ${junction.range} m`);
  assert.ok(night.range < 22.5, `fog pedestrian seen at ${night.range} m`);
  assert.ok(night.t > junction.t);
});

test("each event lands inside its scene and names a real actor when it names one", () => {
  for (const scene of SCENE_IDS.map((id) => buildScene(id)!)) {
    const ids = new Set(scene.actors.map((a) => a.actor_id));
    for (const event of scene.events) {
      assert.ok(event.t >= 0 && event.t <= scene.duration_s, `${scene.scene_id} event at t=${event.t}`);
      for (const mentioned of event.description.match(/\b[a-z]+-\d{2}\b/g) ?? []) {
        assert.ok(ids.has(mentioned), `${scene.scene_id} event names unknown actor ${mentioned}`);
      }
    }
    // Every scene has something worth stopping the playback for.
    assert.ok(scene.events.some((e) => e.severity === "critical"), `${scene.scene_id} has no critical event`);
  }
});

test("the ego brakes in every scene, and speeds stay physically plausible", () => {
  for (const scene of SCENE_IDS.map((id) => buildScene(id)!)) {
    const speeds = scene.ego.track.map((p) => p.speed_mps);
    assert.ok(Math.min(...speeds) < Math.max(...speeds), `${scene.scene_id} never changes speed`);
    assert.ok(Math.max(...speeds) <= 14, `${scene.scene_id} tops ${Math.max(...speeds)} m/s`);
    assert.ok(Math.min(...speeds) >= 0, `${scene.scene_id} goes backwards`);
    let peak = 0;
    for (let i = 1; i < speeds.length; i += 1) peak = Math.max(peak, Math.abs(speeds[i] - speeds[i - 1]) * scene.hz);
    // Nothing here should out-brake a road car on a wet mountain road.
    assert.ok(peak <= 6, `${scene.scene_id} peaks at ${peak} m/s^2`);
  }
});
