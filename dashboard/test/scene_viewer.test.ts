import { test } from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import vm from "node:vm";
import { buildScene } from "../src/scenes";

/**
 * public/scenes.js is browser code with no build step, so it is loaded here in
 * a sandbox with just enough of a DOM to evaluate. Only the pure helpers are
 * exercised — the deck.gl layers need a GPU context and are not testable here.
 */
function loadViewer() {
  const code = readFileSync(new URL("../public/scenes.js", import.meta.url), "utf8");
  const sandbox: Record<string, unknown> = {
    window: {},
    document: { querySelector: () => null },
    requestAnimationFrame: () => 0,
    cancelAnimationFrame: () => undefined,
  };
  sandbox.globalThis = sandbox;
  vm.createContext(sandbox);
  vm.runInContext(code, sandbox);
  return (sandbox.window as { AtlasScenes: any }).AtlasScenes;
}

const viewer = loadViewer();
const { rayBox, scan, posts, footprint, offsetFromRoad, mulberry32 } = viewer.__test;

const scene = (id: string) => {
  const s = buildScene(id)! as any;
  s.postCache = posts(s);
  return s;
};

test("a ray hits a box ahead of it at the right distance", () => {
  const box = { x: 10, y: 0, z: 1, hl: 2, hw: 1, hh: 1, heading: 0 };
  // Straight down +x from the origin: the near face is at x = 8.
  assert.equal(rayBox(0, 0, 1, 1, 0, 0, box), 8);
  // Backwards, and sideways past the box, are both misses.
  assert.equal(rayBox(0, 0, 1, -1, 0, 0, box), Infinity);
  assert.equal(rayBox(0, 5, 1, 1, 0, 0, box), Infinity);
});

test("box rotation is respected", () => {
  // Turned 90 degrees, the long axis now faces the ray, so the near face moves out.
  const turned = { x: 10, y: 0, z: 1, hl: 2, hw: 1, hh: 1, heading: Math.PI / 2 };
  assert.equal(rayBox(0, 0, 1, 1, 0, 0, turned), 9);
});

test("the PRNG is deterministic and stays in range", () => {
  const a = mulberry32(42);
  const b = mulberry32(42);
  for (let i = 0; i < 100; i += 1) {
    const v = a();
    assert.equal(v, b());
    assert.ok(v >= 0 && v < 1);
  }
});

test("a footprint is the box's four corners at its own heading", () => {
  const square = footprint({ x: 0, y: 0, hl: 1, hw: 1, heading: 0 });
  assert.equal(square.length, 4);
  // Compared component-wise: the sandbox's Array is a different realm's, so
  // deepEqual would fail on the prototype rather than on the values.
  const corner = (i: number) => [Math.round(square[i][0]), Math.round(square[i][1])];
  assert.deepEqual(corner(0), [1, 1]);
  assert.deepEqual(corner(2), [-1, -1]);
});

test("lateral offset from the road is signed and near zero on the centreline", () => {
  const s = scene("night_fog_pass");
  const mid = s.road.centerline[20];
  assert.ok(Math.abs(offsetFromRoad(s, mid.x, mid.y)) < 0.5);
  const ego = s.ego.track[0];
  // The ego drives on the right, so its offset is negative and about a lane wide.
  assert.ok(offsetFromRoad(s, ego.x, ego.y) < 0);
});

test("every simulated return lands within the sensor's range and above the road", () => {
  for (const id of ["thimphu_junction", "night_fog_pass"]) {
    const s = scene(id);
    const cloud = scan(s, 60);
    assert.ok(cloud.count > 100, `${id} produced only ${cloud.count} returns`);
    const ego = s.ego.track[60];
    for (let i = 0; i < cloud.count; i += 1) {
      const x = cloud.positions[i * 3];
      const y = cloud.positions[i * 3 + 1];
      const z = cloud.positions[i * 3 + 2];
      assert.ok(Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(z), `${id} non-finite return`);
      assert.ok(z >= 0, `${id} return below the road at z=${z}`);
      // Allow the range noise a little headroom past the nominal range.
      assert.ok(Math.hypot(x - ego.x, y - ego.y) <= s.sensor.lidar_range_m + 2, `${id} return beyond range`);
    }
    assert.equal(cloud.colors.length, cloud.count * 3);
  }
});

test("a scan is reproducible for a given frame and differs between frames", () => {
  const s = scene("dochula_switchback");
  assert.deepEqual(scan(s, 40).positions, scan(s, 40).positions);
  assert.notDeepEqual(scan(s, 40).positions, scan(s, 41).positions);
});

test("fog returns a sparser cloud than clear daylight", () => {
  const clear = scan(scene("thimphu_junction"), 50).count;
  const fog = scan(scene("night_fog_pass"), 50).count;
  assert.ok(fog < clear, `fog returned ${fog}, clear returned ${clear}`);
});

test("a scan stays inside the frame budget for playback", () => {
  const s = scene("thimphu_junction");
  scan(s, 50); // warm the JIT
  const started = performance.now();
  for (let i = 0; i < 5; i += 1) scan(s, 40 + i);
  const per = (performance.now() - started) / 5;
  // Playback advances 10 frames a second, so a scan has ~100 ms before it
  // starts dropping frames. The bound is loose on purpose — it is here to
  // catch a structural regression (an unculled box list, per-ray trig), not to
  // measure the machine.
  assert.ok(per < 100, `${per.toFixed(0)} ms per scan`);
});

test("small roadside objects survive the bounding-sphere rejection", () => {
  const s = scene("night_fog_pass");
  const cloud = scan(s, 100);
  const ego = s.ego.track[100];
  // Verge posts are the smallest targets in the scene and the ones a too-eager
  // cull would drop: expect returns well off the carriageway but close by.
  let verge = 0;
  for (let i = 0; i < cloud.count; i += 1) {
    const x = cloud.positions[i * 3];
    const y = cloud.positions[i * 3 + 1];
    const z = cloud.positions[i * 3 + 2];
    if (z > 0.15 && Math.hypot(x - ego.x, y - ego.y) < 30 && Math.abs(offsetFromRoad(s, x, y)) > 3) verge += 1;
  }
  assert.ok(verge > 5, `only ${verge} returns off the carriageway`);
});

test("actors inside the scan are actually struck by rays", () => {
  const s = scene("thimphu_junction");
  // At t = 13 s the ego is 2.4 m from the parked bus, which is unmissable.
  const cloud = scan(s, 130);
  const bus = s.actors.find((a: any) => a.actor_id === "bus-01");
  const pose = bus.track[130];
  let near = 0;
  for (let i = 0; i < cloud.count; i += 1) {
    if (Math.hypot(cloud.positions[i * 3] - pose.x, cloud.positions[i * 3 + 1] - pose.y) < 6) near += 1;
  }
  assert.ok(near > 20, `only ${near} returns landed on the bus`);
});
