import { test } from "node:test";
import assert from "node:assert/strict";
import { PENALTIES, computeDrivingScore, infractionKind } from "../src/driving_score";

test("a clean completed run scores 100", () => {
  const s = computeDrivingScore([], 60, 60);
  assert.equal(s.driving_score, 100);
  assert.equal(s.route_completion, 100);
  assert.equal(s.infraction_penalty, 1);
  assert.equal(s.route_completion_basis, "duration_vs_planned");
  assert.deepEqual(s.infractions, []);
});

test("penalties multiply and collisions are classified by the other actor", () => {
  const events = [
    { event_class: "collision", severity: "critical", description: "hit walker.pedestrian.0001" },
    { event_class: "collision", severity: "critical", description: "vehicle.tesla.model3" },
    { event_class: "collision", severity: "critical", data: { source_event: "static.prop.rock" } },
    { event_class: "hard_brake", severity: "warning" },
    { event_class: "sensor_dropout", severity: "info" },
  ];
  const s = computeDrivingScore(events, 30, 60);
  const expected = 0.5 * 0.6 * 0.65 * 0.97;
  assert.equal(s.infraction_penalty, Math.round(expected * 10000) / 10000);
  assert.equal(s.route_completion, 50);
  assert.equal(s.driving_score, Math.round(100 * 0.5 * expected * 10000) / 10000);
  assert.equal(s.infractions.length, 4);
  assert.equal(s.infractions[0].kind, "collision_pedestrian");
  assert.equal(infractionKind({ event_class: "sensor_dropout" }), null);
});

test("real-world runs without a plan assume completion", () => {
  const s = computeDrivingScore([{ event_class: "speeding" }], 120, null);
  assert.equal(s.route_completion_basis, "assumed_complete");
  assert.equal(s.driving_score, 100 * PENALTIES.speeding);
});
