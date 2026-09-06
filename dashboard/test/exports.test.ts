import { test } from "node:test";
import assert from "node:assert/strict";
import { McapStreamReader } from "@mcap/core";
import { runToGeoJson, runToMcap, samplesToCsv } from "../src/exports";
import type { RunRow, Sample } from "../src/types";

const run = { run_id: "demo-run-01", tenant_id: "default", source: "vehicle", vehicle_class: "truck", scenario_id: "bt-x", route_id: "r", quality_status: "accepted", distance_km: 1.2, duration_s: 30, started_at: 1 } as unknown as RunRow;
const samples: Sample[] = [
  { t: 1000.0, frame: 0, lat: 27.47, lon: 89.63, alt: 2400, speed_mps: 1, source: "vehicle" },
  { t: 1000.5, frame: 1, lat: 27.471, lon: 89.631, alt: 2401, speed_mps: 2, source: "vehicle" },
];
const events = [
  { event_id: "e1", t: 1000.2, frame: 4, event_class: "hard_brake", severity: "warning", lat: 27.4705, lon: 89.6305, rule_id: "SR-01", description: "brake, \"hard\"" },
  { event_id: "e2", t: 1000.3, event_class: "manual_flag", severity: "info", lat: null, lon: null },
];

test("GeoJSON has a route line and located events only", () => {
  const fc = runToGeoJson(run, samples, events) as { features: Array<{ geometry: { type: string }; properties: Record<string, unknown> }> };
  assert.equal(fc.features.length, 2);
  assert.equal(fc.features[0].geometry.type, "LineString");
  assert.equal(fc.features[1].properties.rule_id, "SR-01");
});

test("CSV quotes cells and keeps column order", () => {
  const csv = samplesToCsv([{ ...samples[0], source: 'a,"b"' }]);
  const [header, row] = csv.trim().split("\n");
  assert.ok(header.startsWith("t,frame,lat,lon,alt,speed_mps"));
  assert.ok(row.endsWith(',"a,""b"""'));
  assert.equal(row.split(",").length, header.split(",").length + 1);
});

test("MCAP export is readable and carries LocationFix, telemetry and log channels", async () => {
  const bytes = await runToMcap(run, samples, events);
  const reader = new McapStreamReader({ validateCrcs: true });
  reader.append(bytes);
  const topics = new Map<number, string>();
  const counts = new Map<string, number>();
  let metadata = 0;
  for (let record = reader.nextRecord(); record; record = reader.nextRecord()) {
    if (record.type === "Channel") topics.set(record.id, record.topic);
    if (record.type === "Message") counts.set(topics.get(record.channelId)!, (counts.get(topics.get(record.channelId)!) || 0) + 1);
    if (record.type === "Metadata") metadata += 1;
  }
  assert.ok(reader.done());
  assert.equal(counts.get("/gnss/fix"), 2);
  assert.equal(counts.get("/atlas/telemetry"), 2);
  assert.equal(counts.get("/atlas/events"), 2);
  assert.equal(metadata, 1);
});
