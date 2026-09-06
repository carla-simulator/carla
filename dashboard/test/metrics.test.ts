import { test } from "node:test";
import assert from "node:assert/strict";
import { renderPrometheus } from "../src/metrics";
import type { KpiReport } from "../src/kpi";

test("Prometheus exposition includes gauges with escaped labels", () => {
  const report: KpiReport = {
    tenant: 'fleet"a',
    computed_at: "2026-09-05T00:00:00Z",
    kpis: [
      { id: "route_coverage", label: "x", value: 12.5, unit: "h", target: "50", status: "at_risk", why: "" },
      { id: "no", label: "x", value: null, unit: "%", target: "", status: "no_data", why: "" },
    ],
    edge_case_rates: [{ event_class: "hard_brake", count: 3, per_100km: 24 }],
    perception: { models: [{ model_id: "m", model_version: "1", evaluations: 1, overall: { tp: 1, fp: 1, fn: 1, precision: 0.5, recall: 0.5, f1: 0.5 }, by_class: {}, by_condition: {} }] },
  };
  const text = renderPrometheus(report, { atlas_fleet_devices: 2 });
  assert.match(text, /atlas_kpi_value\{tenant="fleet\\"a",kpi="route_coverage",unit="h"\} 12.5/);
  assert.match(text, /atlas_kpi_on_track\{tenant="fleet\\"a",kpi="route_coverage"\} 0/);
  assert.doesNotMatch(text, /kpi="no"/);
  assert.match(text, /atlas_edge_case_rate_per_100km\{[^}]*event_class="hard_brake"\} 24/);
  assert.match(text, /atlas_perception_metric\{[^}]*metric="f1"\} 0.5/);
  assert.match(text, /atlas_fleet_devices\{tenant="fleet\\"a"\} 2/);
  assert.match(text, /atlas_kpi_computed_timestamp_seconds\{[^}]*\} 1788566400/);
});
