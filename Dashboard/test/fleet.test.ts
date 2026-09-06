import { test } from "node:test";
import assert from "node:assert/strict";
import { fromOsmAnd, fromTraccar, positionsToSamples } from "../src/routes/fleet";

test("Traccar forward payload maps to a position (knots to m/s)", () => {
  const p = fromTraccar({
    device: { id: 7, uniqueId: "BT-TRUCK-01", name: "Truck 1", category: "truck" },
    position: { deviceId: 7, protocol: "osmand", fixTime: "2026-09-05T10:00:00.000+00:00", latitude: 27.47, longitude: 89.63, altitude: 2400, speed: 10, course: 90, attributes: { batteryLevel: 80 } },
    event: { type: "deviceOnline" },
  });
  assert.equal(p.device_id, "BT-TRUCK-01");
  assert.equal(p.t, Date.parse("2026-09-05T10:00:00Z") / 1000);
  assert.ok(Math.abs((p.speed_mps || 0) - 5.14444) < 1e-3);
  assert.equal(p.heading_deg, 90);
  assert.equal((p.attributes as Record<string, unknown>).event, "deviceOnline");
  assert.throws(() => fromTraccar({ position: { latitude: 1 } }), /device id/);
});

test("OsmAnd query maps to a position with epoch seconds or milliseconds", () => {
  const p = fromOsmAnd(new URLSearchParams("id=phone-1&lat=27.5&lon=89.6&timestamp=1788602400&speed=2&bearing=45&altitude=2300&batt=55"));
  assert.equal(p.device_id, "phone-1");
  assert.equal(p.t, 1788602400);
  assert.equal(p.alt, 2300);
  assert.equal((p.attributes as Record<string, unknown>).batt, 55);
  assert.equal(fromOsmAnd(new URLSearchParams("id=x&lat=1&lon=2&timestamp=1788602400000")).t, 1788602400);
  assert.throws(() => fromOsmAnd(new URLSearchParams("lat=1&lon=2")), /id is required/);
});

test("positionsToSamples derives speed, heading, acceleration and grade", () => {
  const rows = [
    { t: 0, lat: 27.0, lon: 89.0, alt: 1000, speed_mps: null, heading_deg: null, accuracy_m: null, source: "generic", attributes: null },
    { t: 10, lat: 27.0009, lon: 89.0, alt: 1005, speed_mps: null, heading_deg: null, accuracy_m: null, source: "generic", attributes: null },
  ];
  const s = positionsToSamples(rows);
  assert.equal(s.length, 2);
  assert.ok(Math.abs((s[1].speed_mps || 0) - 10) < 0.2);       // ~100 m in 10 s
  assert.ok(Math.abs((s[1].heading_deg || 0) - 0) < 1e-6);      // due north
  assert.ok(Math.abs((s[1].accel_x || 0) - 1) < 0.05);
  assert.ok(Math.abs((s[1].grade_pct || 0) - 5) < 0.2);
  assert.equal(s[1].source, "vehicle");
});
