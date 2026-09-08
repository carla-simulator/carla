import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { DEFAULT_INPUT, GPUS, PROGRAMS, RESOLUTIONS, TIERS, comparePlans, planProgram } from "../planner";
import { json, readJson } from "../util";

/** Everything already in the catalog, expressed in the planner's units. */
async function catalogCoverage(c: RouteContext, tenant: string, clipSeconds: number) {
  const [runs, scenarios, clips] = await Promise.all([
    c.env.DB.prepare(
      "SELECT COUNT(*) AS runs, COALESCE(SUM(CASE WHEN source = 'vehicle' THEN duration_s ELSE 0 END), 0) AS vehicle_s, COALESCE(SUM(CASE WHEN source = 'sim' THEN duration_s ELSE 0 END), 0) AS sim_s, COALESCE(SUM(CASE WHEN quality_status = 'accepted' THEN duration_s ELSE 0 END), 0) AS accepted_s FROM runs WHERE tenant_id = ?1",
    ).bind(tenant).first<{ runs: number; vehicle_s: number; sim_s: number; accepted_s: number }>(),
    c.env.DB.prepare("SELECT COUNT(*) AS variants, COUNT(DISTINCT family) AS families FROM scenarios WHERE tenant_id = ?1").bind(tenant).first<{ variants: number; families: number }>(),
    c.env.DB.prepare("SELECT COUNT(*) AS clips, COALESCE(SUM(size_bytes), 0) AS bytes FROM clips WHERE tenant_id = ?1").bind(tenant).first<{ clips: number; bytes: number }>(),
  ]);
  const acceptedSeconds = runs?.accepted_s || 0;
  return {
    runs: runs?.runs || 0,
    vehicle_hours: Math.round(((runs?.vehicle_s || 0) / 3600) * 10) / 10,
    sim_hours: Math.round(((runs?.sim_s || 0) / 3600) * 10) / 10,
    accepted_hours: Math.round((acceptedSeconds / 3600) * 10) / 10,
    scenario_variants: scenarios?.variants || 0,
    scenario_families: scenarios?.families || 0,
    clips: clips?.clips || 0,
    clip_gb: Math.round(((clips?.bytes || 0) / 1e9) * 100) / 100,
    /** Accepted collection time cut into clips of the planned length. */
    equivalent_scenes: Math.floor(acceptedSeconds / clipSeconds),
  };
}

function withGap(plan: ReturnType<typeof planProgram>, coverage: Awaited<ReturnType<typeof catalogCoverage>>) {
  const target = plan.input.scenes;
  const have = coverage.equivalent_scenes;
  return {
    ...coverage,
    target_scenes: target,
    scenes_remaining: Math.max(0, target - have),
    progress_pct: target > 0 ? Math.round((Math.min(have, target) / target) * 1000) / 10 : 0,
    collection_hours_remaining: Math.round((Math.max(0, target - have) * plan.input.clip_seconds) / 3600),
  };
}

/** GET /api/planner/options — the reference tables the form and any client build on. */
export function getPlannerOptions(_c: RouteContext): Response {
  const list = <T extends Record<string, unknown>>(table: Record<string, T>) => Object.entries(table).map(([id, value]) => ({ id, ...value }));
  return json({ defaults: DEFAULT_INPUT, tiers: TIERS, programs: list(PROGRAMS), gpus: list(GPUS), resolutions: list(RESOLUTIONS) }, 200, { "cache-control": "public, max-age=300" });
}

/** GET /api/planner?scenes=100000&clip_seconds=10&gpu=a100_80gb&program=fine_tune */
export async function getPlanner(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const raw = Object.fromEntries(c.url.searchParams.entries());
  const plan = planProgram(raw);
  const coverage = await catalogCoverage(c, who.tenant, plan.input.clip_seconds);
  return json({ ...plan, compare: comparePlans(plan.input), coverage: withGap(plan, coverage) });
}

/** POST /api/planner — same plan from a JSON body, for scripts and notebooks. */
export async function postPlanner(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const body = await readJson<Record<string, unknown>>(c.request, 64 * 1024);
  const plan = planProgram(body || {});
  const coverage = await catalogCoverage(c, who.tenant, plan.input.clip_seconds);
  return json({ ...plan, compare: comparePlans(plan.input), coverage: withGap(plan, coverage) });
}
