import { test } from "node:test";
import assert from "node:assert/strict";
import { scenarioToXosc } from "../src/openscenario";

const scenario = {
  scenario_id: "bt-landslide-debris-01",
  family: "landslide_debris",
  name: "Landslide debris after a curve",
  description: 'Debris & "rocks" <on> the road',
  content_hash: "sha256:abc",
  params: { weather_preset: "heavy_rain", time_of_day: "dusk", speed_limit_kph: 30, vehicle_class: "truck", duration_s: 45, road_curvature: 0.05, grade_pct: -7, payload_kg: 4000, sensor_degradation: { camera_blur: 0.4, gnss_noise_m: 2.5 } },
  actors: [
    { role: "debris", lane: "ego", distance_m: 60, lateral_m: 0.5 },
    { role: "oncoming_truck", lane: "opposite", distance_m: 80, speed_mps: 9 },
    { role: "lead_vehicle_stopping", lane: "ego", distance_m: 40, speed_mps: 8 },
    { role: "pedestrian", lane: "shoulder", distance_m: 25 },
  ],
  expected_events: ["obstacle_ahead", "hard_brake"],
};

test("scenarioToXosc produces an OpenSCENARIO 1.2 document with entities, weather and triggers", () => {
  const xml = scenarioToXosc(scenario, new Date("2026-09-05T10:00:00Z"));
  assert.ok(xml.startsWith('<?xml version="1.0" encoding="UTF-8"?>\n<OpenSCENARIO'));
  assert.match(xml, /revMajor="1" revMinor="2"/);
  assert.match(xml, /Debris &amp; &quot;rocks&quot; &lt;on&gt; the road/);
  assert.match(xml, /<ScenarioObject name="hero">[\s\S]*vehicleCategory="truck" mass="16000"/);
  assert.match(xml, /<MiscObject name="static.prop.rock"/);
  assert.match(xml, /<Pedestrian name="walker.pedestrian.0001"/);
  assert.match(xml, /entityRef="oncoming_truck_2"[\s\S]*dLane="\$oncoming_dlane" ds="80.00"/);
  assert.match(xml, /AbsoluteTargetSpeed value="9.00"/);
  assert.match(xml, /<Precipitation precipitationType="rain" intensity="0.8"\/>/);
  assert.match(xml, /<Fog visualRange="400"\/>/);
  assert.match(xml, /dateTime="2026-09-05T18:15:00"/);
  assert.match(xml, /ParameterDeclaration name="sensor_camera_blur" parameterType="double" value="0.4"/);
  assert.match(xml, /name="lead_vehicle_stopping_3_brake"/);
  assert.match(xml, /SimulationTimeCondition value="\$scenario_duration"/);
  // Well-formed: every opened tag closes.
  const opened = (xml.match(/<([A-Za-z]+)[\s>/]/g) || []).length;
  const closed = (xml.match(/<\/([A-Za-z]+)>/g) || []).length + (xml.match(/\/>/g) || []).length;
  assert.equal(opened, closed);
});
