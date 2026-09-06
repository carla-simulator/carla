import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { DATASET_TIERS, GPU_CATALOG, WORKLOADS, coverageChecklist, estimateBudget, type BudgetInput, type CoverageRow } from "../budget";
import { json } from "../util";

const INPUT_KEYS: Array<keyof BudgetInput> = ["scenes", "clip_seconds", "fps", "cameras", "width", "height", "workload", "gpu", "price_per_hour", "interruptible", "parallel_gpus", "storage_price_gb_month"];

/**
 * GET /api/budget/estimate — low/mid/high storage, GPU-hour and cost plan for a
 * synthetic-data program, plus a coverage checklist of the tenant's scenario
 * library and runs so the plan says what to generate next, not just what it costs.
 */
export async function getBudgetEstimate(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const raw: Partial<Record<keyof BudgetInput, unknown>> = {};
  for (const key of INPUT_KEYS) {
    const v = c.url.searchParams.get(key);
    if (v !== null) raw[key] = v;
  }
  const [scenarioRows, runRows, scenarioCount] = await Promise.all([
    c.env.DB.prepare("SELECT lighting_class, visibility_class, route_class, COUNT(*) AS n FROM scenarios WHERE tenant_id = ?1 GROUP BY lighting_class, visibility_class, route_class").bind(who.tenant).all<CoverageRow>(),
    c.env.DB.prepare("SELECT s.lighting_class, s.visibility_class, s.route_class, COUNT(*) AS n FROM runs r LEFT JOIN scenarios s ON s.scenario_id = r.scenario_id AND s.tenant_id = r.tenant_id WHERE r.tenant_id = ?1 GROUP BY s.lighting_class, s.visibility_class, s.route_class").bind(who.tenant).all<CoverageRow>(),
    c.env.DB.prepare("SELECT COUNT(*) AS n FROM scenarios WHERE tenant_id = ?1").bind(who.tenant).first<{ n: number }>(),
  ]);
  const estimate = estimateBudget(raw as Partial<BudgetInput>);
  return json({
    ...estimate,
    coverage: {
      scenarios: coverageChecklist(scenarioRows.results),
      runs: coverageChecklist(runRows.results),
      library_scenarios: scenarioCount?.n || 0,
    },
    catalog: {
      gpus: Object.entries(GPU_CATALOG).map(([id, g]) => ({ id, ...g })),
      workloads: Object.entries(WORKLOADS).map(([id, w]) => ({ id, ...w })),
      tiers: DATASET_TIERS.map((t) => ({ id: t.id, label: t.label, range: t.range, note: t.note })),
    },
  });
}
