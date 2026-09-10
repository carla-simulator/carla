import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { json } from "../util";
import { buildCoverage, type CoverageMatrix, type RunCoverageRow, type ScenarioCoverageRow } from "../coverage";
import type { Env } from "../types";

/** Catalog counts alongside the matrix, so a client can size a plan from them. */
export interface CatalogBasis {
  /** Accepted runs — the unit a "scene" is counted in. */
  accepted_runs: number;
  runs: number;
  scenarios: number;
  clips: number;
  /** Mean released-clip size, when clips carry bytes; drives MB per scene. */
  mb_per_clip: number | null;
  /** Mean accepted-run duration, when known; drives seconds per scene. */
  seconds_per_run: number | null;
}

/** Coverage matrix plus the catalog counts a planner run can default from. */
export async function coverageFor(env: Env, tenant: string, minRunsPerCell: number): Promise<{ coverage: CoverageMatrix; catalog: CatalogBasis }> {
  const [scenarioRows, runRows, clips, runStats] = await Promise.all([
    env.DB.prepare(
      "SELECT route_class, lighting_class, visibility_class, review_status, COUNT(*) AS variants FROM scenarios WHERE tenant_id = ?1 GROUP BY route_class, lighting_class, visibility_class, review_status",
    ).bind(tenant).all<ScenarioCoverageRow>(),
    env.DB.prepare(
      "SELECT s.route_class AS route_class, s.lighting_class AS lighting_class, s.visibility_class AS visibility_class, r.source AS source, r.quality_status AS quality_status," +
        " COUNT(*) AS runs, COALESCE(SUM(r.duration_s), 0) AS duration_s, COALESCE(SUM(r.distance_km), 0) AS distance_km" +
        " FROM runs r LEFT JOIN scenarios s ON s.scenario_id = r.scenario_id AND s.tenant_id = r.tenant_id" +
        " WHERE r.tenant_id = ?1 GROUP BY 1, 2, 3, 4, 5",
    ).bind(tenant).all<RunCoverageRow>(),
    env.DB.prepare("SELECT COUNT(*) AS n, AVG(NULLIF(size_bytes, 0)) AS avg_bytes FROM clips WHERE tenant_id = ?1").bind(tenant).first<{ n: number; avg_bytes: number | null }>(),
    env.DB.prepare("SELECT COUNT(*) AS n, AVG(NULLIF(duration_s, 0)) AS avg_duration FROM runs WHERE tenant_id = ?1 AND quality_status = 'accepted'").bind(tenant).first<{ n: number; avg_duration: number | null }>(),
  ]);

  const coverage = buildCoverage(scenarioRows.results, runRows.results, minRunsPerCell);
  return {
    coverage,
    catalog: {
      accepted_runs: runStats?.n || 0,
      runs: coverage.totals.runs,
      scenarios: coverage.totals.scenarios,
      clips: clips?.n || 0,
      mb_per_clip: clips?.avg_bytes ? Math.round((clips.avg_bytes / 1e6) * 100) / 100 : null,
      seconds_per_run: runStats?.avg_duration ? Math.round(runStats.avg_duration * 10) / 10 : null,
    },
  };
}

function minRunsParam(c: RouteContext): number {
  const n = Number(c.url.searchParams.get("min_runs") || 3);
  return Number.isFinite(n) ? Math.min(100, Math.max(1, Math.round(n))) : 3;
}

/** GET /api/coverage — ODD coverage matrix (visibility x lighting) and gaps. */
export async function getCoverage(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const { coverage, catalog } = await coverageFor(c.env, who.tenant, minRunsParam(c));
  return json({ tenant: who.tenant, computed_at: new Date().toISOString(), catalog, ...coverage });
}
