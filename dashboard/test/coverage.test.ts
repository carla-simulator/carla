import { test } from "node:test";
import assert from "node:assert/strict";
import { buildCoverage } from "../src/coverage";

const scenario = (visibility: string, lighting: string, route: string, variants = 1, review = "unreviewed") => ({
  visibility_class: visibility,
  lighting_class: lighting,
  route_class: route,
  review_status: review,
  variants,
});

test("the coverage matrix crosses every visibility with every lighting class", () => {
  const cov = buildCoverage(
    [scenario("clear", "daylight", "straight_flat", 4, "reviewed"), scenario("fog", "night", "hairpin_flat", 2)],
    [],
  );
  assert.deepEqual(cov.axes.visibility, ["clear", "fog"]);
  assert.deepEqual(cov.axes.lighting, ["daylight", "night"]);
  assert.equal(cov.cells.length, 4);
  assert.equal(cov.totals.scenarios, 6);
  // Variants without runs are thin, empty crossings are gaps.
  const clearDay = cov.cells.find((c) => c.visibility === "clear" && c.lighting === "daylight")!;
  assert.equal(clearDay.status, "thin");
  assert.equal(clearDay.reviewed, 4);
  assert.equal(cov.cells.find((c) => c.visibility === "clear" && c.lighting === "night")!.status, "gap");
  assert.equal(cov.totals.gap_cells, 2);
  assert.equal(cov.totals.coverage_pct, 0);
});

test("a cell turns covered once it has enough accepted runs", () => {
  const runs = [
    { visibility_class: "clear", lighting_class: "daylight", route_class: "straight_flat", quality_status: "accepted", runs: 3, duration_s: 180, distance_km: 12 },
    { visibility_class: "clear", lighting_class: "daylight", route_class: "straight_flat", quality_status: "rejected", runs: 1, duration_s: 60, distance_km: 4 },
  ];
  const cov = buildCoverage([scenario("clear", "daylight", "straight_flat", 5)], runs);
  const cell = cov.cells[0];
  assert.equal(cell.runs, 4);
  assert.equal(cell.accepted_runs, 3);
  assert.equal(cell.status, "covered");
  assert.equal(cov.totals.covered_cells, 1);
  assert.equal(cov.totals.coverage_pct, 100);
  assert.deepEqual(cov.gaps, []);
  assert.equal(cov.by_route_class[0].duration_s, 240);
  // Raising the bar per cell takes the same data back to thin.
  assert.equal(buildCoverage([scenario("clear", "daylight", "straight_flat", 5)], runs, 5).cells[0].status, "thin");
});

test("runs without a scenario land in an unlabelled bucket rather than being dropped", () => {
  const cov = buildCoverage(
    [scenario("clear", "daylight", "straight_flat")],
    [{ visibility_class: null, lighting_class: null, route_class: null, quality_status: "accepted", runs: 2, duration_s: 100, distance_km: 8 }],
  );
  assert.ok(cov.axes.visibility.includes("unlabelled"));
  assert.equal(cov.axes.visibility[cov.axes.visibility.length - 1], "unlabelled");
  assert.equal(cov.totals.runs, 2);
  assert.equal(cov.cells.find((c) => c.visibility === "unlabelled" && c.lighting === "unlabelled")!.accepted_runs, 2);
  assert.equal(cov.by_route_class.find((r) => r.route_class === "unlabelled")!.runs, 2);
  // The unlabelled row and column are shown but are not ODD cells, so the only
  // cell counted here is clear x daylight, which has variants but no runs.
  assert.equal(cov.cells.length, 4);
  assert.equal(cov.totals.cells, 1);
  assert.equal(cov.totals.thin_cells, 1);
  assert.equal(cov.totals.gap_cells, 0);
  assert.deepEqual(cov.gaps.map((g) => `${g.visibility}/${g.lighting}`), ["clear/daylight"]);
});

test("gaps are ordered worst first", () => {
  const cov = buildCoverage(
    [scenario("clear", "daylight", "straight_flat", 9), scenario("fog", "daylight", "curve_flat", 1), scenario("fog", "night", "hairpin_flat", 1)],
    [{ visibility_class: "clear", lighting_class: "daylight", route_class: "straight_flat", quality_status: "accepted", runs: 2, duration_s: 120, distance_km: 8 }],
  );
  // Empty cells first, then cells with variants but no runs (ties keep axis
  // order), then the thin cell that already has some accepted runs.
  assert.deepEqual(cov.gaps.map((g) => `${g.visibility}/${g.lighting}`), ["clear/night", "fog/daylight", "fog/night", "clear/daylight"]);
});
