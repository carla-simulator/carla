import type { RouteContext } from "../router";
import type { PlanTotals, SavedPlan } from "../plans";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { comparePlanRecords, hydrate, planToCsv, planToMarkdown, slugPlanId, summarize } from "../plans";
import { normalizeInput, planProgram } from "../planner";
import { HttpError, json, nowIso, optionalString, parseJsonColumn, readJson, requireId, requireString } from "../util";

interface PlanRow {
  plan_id: string;
  name: string;
  notes: string | null;
  input: string;
  saved_totals: string;
  created_by: string | null;
  created_at: string;
  updated_at: string | null;
}

function inflate(row: PlanRow): SavedPlan {
  return hydrate({
    plan_id: row.plan_id,
    name: row.name,
    notes: row.notes,
    input: normalizeInput(parseJsonColumn<Record<string, unknown>>(row.input, {})),
    saved: parseJsonColumn<PlanTotals>(row.saved_totals, summarize(planProgram({}))),
    created_by: row.created_by,
    created_at: row.created_at,
    updated_at: row.updated_at,
  });
}

async function loadPlan(c: RouteContext, tenant: string, id: string): Promise<SavedPlan> {
  const row = await c.env.DB.prepare("SELECT plan_id, name, notes, input, saved_totals, created_by, created_at, updated_at FROM plans WHERE plan_id = ?1 AND tenant_id = ?2")
    .bind(id, tenant).first<PlanRow>();
  if (!row) throw new HttpError(404, "plan not found");
  return inflate(row);
}

/** GET /api/plans — every saved plan, priced at today's rates with its drift. */
export async function listPlans(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const rows = await c.env.DB.prepare("SELECT plan_id, name, notes, input, saved_totals, created_by, created_at, updated_at FROM plans WHERE tenant_id = ?1 ORDER BY created_at DESC LIMIT 200")
    .bind(who.tenant).all<PlanRow>();
  return json({ plans: rows.results.map(inflate) });
}

/**
 * POST /api/plans — save (or overwrite) a named plan.
 * The body is a planner input plus `name`; the totals are computed here so a
 * client can never store numbers the model would not produce.
 */
export async function savePlan(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<Record<string, unknown> & { name?: string; plan_id?: string; notes?: string }>(c.request, 64 * 1024);
  const name = requireString(body.name, "name", 120);
  const id = body.plan_id ? requireId(body.plan_id, "plan_id") : slugPlanId(name);
  // Plan ids are slugs, so two tenants can want the same one; the upsert below is
  // tenant-guarded and would silently no-op, which reads as a lost save.
  const owner = await c.env.DB.prepare("SELECT tenant_id FROM plans WHERE plan_id = ?1").bind(id).first<{ tenant_id: string }>();
  if (owner && owner.tenant_id !== who.tenant) throw new HttpError(409, "plan_id is already taken", { plan_id: id });
  const plan = planProgram(body);
  const totals = summarize(plan);
  await c.env.DB.prepare(
    `INSERT INTO plans (plan_id, tenant_id, name, notes, input, saved_totals, created_by)
     VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)
     ON CONFLICT(plan_id) DO UPDATE SET name = excluded.name, notes = excluded.notes, input = excluded.input,
        saved_totals = excluded.saved_totals, updated_at = ?8
     WHERE plans.tenant_id = excluded.tenant_id`,
  ).bind(id, who.tenant, name, optionalString(body.notes, 2000), JSON.stringify(plan.input), JSON.stringify(totals), who.actor, nowIso()).run();
  await audit(c.env, who, "plan.save", id, { name, scenes: plan.input.scenes, total_usd: totals.total_usd });
  return json({ plan: await loadPlan(c, who.tenant, id) }, 201);
}

/**
 * GET /api/plans/diff?a=<plan_id>&b=<plan_id> — what changed between two
 * budgets: the inputs that differ and what they did to the numbers.
 */
export async function diffPlans(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const aId = requireId(c.url.searchParams.get("a"), "a");
  const bId = requireId(c.url.searchParams.get("b"), "b");
  const [a, b] = await Promise.all([loadPlan(c, who.tenant, aId), loadPlan(c, who.tenant, bId)]);
  return json(comparePlanRecords(a, b));
}

/** GET /api/plans/:id — one plan with the full recomputed detail behind it. */
export async function getPlan(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const record = await loadPlan(c, who.tenant, requireId(c.params.id, "plan_id"));
  return json({ plan: record, detail: planProgram(record.input) });
}

/** DELETE /api/plans/:id */
export async function deletePlan(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const id = requireId(c.params.id, "plan_id");
  const result = await c.env.DB.prepare("DELETE FROM plans WHERE plan_id = ?1 AND tenant_id = ?2").bind(id, who.tenant).run();
  if (!result.meta.changes) throw new HttpError(404, "plan not found");
  await audit(c.env, who, "plan.delete", id);
  return json({ plan_id: id, deleted: true });
}

/** GET /api/plans/:id/export/:format — csv for a spreadsheet, md for the partner report. */
export async function exportPlan(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const record = await loadPlan(c, who.tenant, requireId(c.params.id, "plan_id"));
  const plan = planProgram(record.input);
  const format = c.params.format;
  if (format === "csv") {
    return new Response(planToCsv(record, plan), {
      headers: { "content-type": "text/csv; charset=utf-8", "content-disposition": `attachment; filename="${record.plan_id}-budget.csv"`, "cache-control": "no-store" },
    });
  }
  if (format === "md" || format === "markdown") {
    return new Response(planToMarkdown(record, plan), {
      headers: { "content-type": "text/markdown; charset=utf-8", "content-disposition": `attachment; filename="${record.plan_id}-budget.md"`, "cache-control": "no-store" },
    });
  }
  throw new HttpError(400, "format must be csv or md");
}
