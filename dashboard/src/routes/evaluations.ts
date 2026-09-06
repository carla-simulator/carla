import type { RouteContext } from "../router";
import type { Counts } from "../types";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { HttpError, json, nowIso, optionalNumber, optionalString, parseJsonColumn, readJson, requireId, requireString } from "../util";

interface EvaluationIn {
  evaluation_id: string;
  model_id: string;
  model_version?: string;
  run_id?: string;
  scenario_id?: string | null;
  conditions?: Record<string, string>;
  overall: Counts;
  by_class?: Record<string, Counts>;
  by_condition?: Record<string, Record<string, Counts>>;
  failure_clusters?: unknown[];
  frames_evaluated?: number;
  inputs?: Record<string, string>;
  reproducible?: boolean;
}

function inflate(row: Record<string, unknown>): Record<string, unknown> {
  return {
    ...row,
    conditions: parseJsonColumn(row.conditions, {}),
    overall: parseJsonColumn(row.overall, {}),
    by_class: parseJsonColumn(row.by_class, {}),
    by_condition: parseJsonColumn(row.by_condition, {}),
    failure_clusters: parseJsonColumn(row.failure_clusters, []),
    inputs: parseJsonColumn(row.inputs, {}),
  };
}

export async function listEvaluations(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const model = c.url.searchParams.get("model_id");
  const run = c.url.searchParams.get("run_id");
  const binds: unknown[] = [who.tenant];
  const where = ["tenant_id = ?1"];
  if (model) { binds.push(model); where.push(`model_id = ?${binds.length}`); }
  if (run) { binds.push(run); where.push(`run_id = ?${binds.length}`); }
  const rows = await c.env.DB.prepare(`SELECT * FROM evaluations WHERE ${where.join(" AND ")} ORDER BY created_at DESC LIMIT 500`).bind(...binds).all<Record<string, unknown>>();
  return json({ evaluations: rows.results.map(inflate) });
}

export async function getEvaluation(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const id = requireId(c.params.id, "evaluation_id");
  const row = await c.env.DB.prepare("SELECT * FROM evaluations WHERE evaluation_id = ?1 AND tenant_id = ?2").bind(id, who.tenant).first<Record<string, unknown>>();
  if (!row) throw new HttpError(404, "evaluation not found");
  return json({ evaluation: inflate(row) });
}

export async function createEvaluation(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<EvaluationIn>(c.request);
  const id = requireId(body.evaluation_id, "evaluation_id");
  const model = requireString(body.model_id, "model_id", 100);
  if (!body.overall || typeof body.overall.tp !== "number") throw new HttpError(400, "overall counts are required");
  const inputs = body.inputs || {};
  const reproducible = body.reproducible ?? Boolean(inputs.ground_truth && inputs.detections);
  await c.env.DB.prepare(
    `INSERT INTO evaluations (evaluation_id, tenant_id, model_id, model_version, run_id, scenario_id, conditions, overall, by_class, by_condition,
        failure_clusters, frames_evaluated, inputs, reproducible)
     VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14)
     ON CONFLICT(evaluation_id) DO UPDATE SET overall = excluded.overall, by_class = excluded.by_class, by_condition = excluded.by_condition,
        failure_clusters = excluded.failure_clusters, frames_evaluated = excluded.frames_evaluated, inputs = excluded.inputs, reproducible = excluded.reproducible
     WHERE evaluations.tenant_id = excluded.tenant_id`,
  ).bind(id, who.tenant, model, optionalString(body.model_version, 64), optionalString(body.run_id, 200), optionalString(body.scenario_id, 200),
    JSON.stringify(body.conditions || {}), JSON.stringify(body.overall), JSON.stringify(body.by_class || {}), JSON.stringify(body.by_condition || {}),
    JSON.stringify((body.failure_clusters || []).slice(0, 200)), optionalNumber(body.frames_evaluated) ?? 0, JSON.stringify(inputs), reproducible ? 1 : 0).run();
  await audit(c.env, who, "evaluation.upsert", id, { model_id: model, run_id: body.run_id, reproducible });
  return json({ evaluation_id: id, reproducible }, 201);
}

export async function markReplayVerified(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const id = requireId(c.params.id, "evaluation_id");
  const body = await readJson<{ replay_evaluation_id?: string; matches: boolean }>(c.request);
  const original = await c.env.DB.prepare("SELECT inputs, overall FROM evaluations WHERE evaluation_id = ?1 AND tenant_id = ?2").bind(id, who.tenant).first<{ inputs: string; overall: string }>();
  if (!original) throw new HttpError(404, "evaluation not found");
  let matches = Boolean(body.matches);
  if (body.replay_evaluation_id) {
    const replay = await c.env.DB.prepare("SELECT inputs, overall FROM evaluations WHERE evaluation_id = ?1 AND tenant_id = ?2").bind(body.replay_evaluation_id, who.tenant).first<{ inputs: string; overall: string }>();
    if (!replay) throw new HttpError(404, "replay evaluation not found");
    matches = replay.inputs === original.inputs && replay.overall === original.overall;
  }
  await c.env.DB.prepare("UPDATE evaluations SET replay_verified = ?3 WHERE evaluation_id = ?1 AND tenant_id = ?2").bind(id, who.tenant, matches ? 1 : 0).run();
  await audit(c.env, who, "evaluation.replay_verified", id, { matches, replay_evaluation_id: body.replay_evaluation_id, at: nowIso() });
  return json({ evaluation_id: id, replay_verified: matches });
}
