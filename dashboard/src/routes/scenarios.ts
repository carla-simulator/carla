import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { HttpError, json, nowIso, optionalNumber, optionalString, parseJsonColumn, readJson, requireId, requireString } from "../util";

interface ScenarioIn {
  id: string;
  family: string;
  group?: string;
  name?: string;
  description?: string;
  tags?: string[];
  params: Record<string, unknown>;
  actors?: unknown[];
  expected_events?: string[];
  seed?: number;
  version?: string;
  content_hash: string;
  route_class?: string;
  lighting_class?: string;
  visibility_class?: string;
  review_status?: string;
}

function inflate(row: Record<string, unknown>): Record<string, unknown> {
  return {
    ...row,
    tags: parseJsonColumn(row.tags, []),
    params: parseJsonColumn(row.params, {}),
    actors: parseJsonColumn(row.actors, []),
    expected_events: parseJsonColumn(row.expected_events, []),
  };
}

export async function listScenarios(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const family = c.url.searchParams.get("family");
  const group = c.url.searchParams.get("group");
  const review = c.url.searchParams.get("review_status");
  const binds: unknown[] = [who.tenant];
  const where = ["tenant_id = ?1"];
  if (family) { binds.push(family); where.push(`family = ?${binds.length}`); }
  if (group) { binds.push(group); where.push(`group_name = ?${binds.length}`); }
  if (review) { binds.push(review); where.push(`review_status = ?${binds.length}`); }
  const rows = await c.env.DB.prepare(`SELECT * FROM scenarios WHERE ${where.join(" AND ")} ORDER BY family, scenario_id LIMIT 1000`).bind(...binds).all();
  const families = await c.env.DB.prepare(
    "SELECT family, group_name, COUNT(*) AS variants, SUM(CASE WHEN review_status = 'reviewed' THEN 1 ELSE 0 END) AS reviewed FROM scenarios WHERE tenant_id = ?1 GROUP BY family, group_name ORDER BY group_name, family",
  ).bind(who.tenant).all();
  return json({ scenarios: rows.results.map(inflate), families: families.results });
}

export async function getScenario(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const id = requireId(c.params.id, "scenario_id");
  const row = await c.env.DB.prepare("SELECT * FROM scenarios WHERE scenario_id = ?1 AND tenant_id = ?2").bind(id, who.tenant).first<Record<string, unknown>>();
  if (!row) throw new HttpError(404, "scenario not found");
  const runs = await c.env.DB.prepare("SELECT run_id, started_at, quality_status, event_count, distance_km FROM runs WHERE scenario_id = ?1 AND tenant_id = ?2 ORDER BY started_at DESC LIMIT 50").bind(id, who.tenant).all();
  return json({ scenario: inflate(row), runs: runs.results });
}

export async function importScenarios(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<{ scenarios: ScenarioIn[]; taxonomy_version?: string }>(c.request);
  const items = Array.isArray(body.scenarios) ? body.scenarios : [];
  if (items.length === 0) throw new HttpError(400, "scenarios[] is empty");
  if (items.length > 2000) throw new HttpError(413, "at most 2000 scenarios per import");
  const statements: D1PreparedStatement[] = [];
  for (const s of items) {
    const id = requireId(s.id, "scenario id");
    const family = requireString(s.family, "family", 100);
    const hash = requireString(s.content_hash, "content_hash", 100);
    statements.push(
      c.env.DB.prepare(
        `INSERT INTO scenarios (scenario_id, tenant_id, family, group_name, name, description, tags, params, actors, expected_events, seed, version,
            content_hash, route_class, lighting_class, visibility_class, review_status, updated_at)
         VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18)
         ON CONFLICT(scenario_id) DO UPDATE SET family = excluded.family, group_name = excluded.group_name, name = excluded.name,
            description = excluded.description, tags = excluded.tags, params = excluded.params, actors = excluded.actors,
            expected_events = excluded.expected_events, seed = excluded.seed, version = excluded.version,
            route_class = excluded.route_class, lighting_class = excluded.lighting_class, visibility_class = excluded.visibility_class,
            -- A content change invalidates an earlier expert review.
            review_status = CASE WHEN scenarios.content_hash = excluded.content_hash THEN scenarios.review_status ELSE 'unreviewed' END,
            content_hash = excluded.content_hash, updated_at = excluded.updated_at
         WHERE scenarios.tenant_id = excluded.tenant_id`,
      ).bind(id, who.tenant, family, optionalString(s.group, 100), optionalString(s.name, 300) || id, optionalString(s.description, 4000),
        JSON.stringify(s.tags || []), JSON.stringify(s.params || {}), JSON.stringify(s.actors || []), JSON.stringify(s.expected_events || []),
        optionalNumber(s.seed), optionalString(s.version, 32), hash, optionalString(s.route_class, 64), optionalString(s.lighting_class, 32),
        optionalString(s.visibility_class, 32), s.review_status === "reviewed" ? "reviewed" : "unreviewed", nowIso()),
    );
  }
  // D1 batches are limited in size; send in slices.
  for (let i = 0; i < statements.length; i += 100) await c.env.DB.batch(statements.slice(i, i + 100));
  await audit(c.env, who, "scenarios.import", null, { count: items.length, taxonomy_version: body.taxonomy_version });
  return json({ imported: items.length }, 201);
}

export async function reviewScenario(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const id = requireId(c.params.id, "scenario_id");
  const body = await readJson<{ review_status: string; notes?: string }>(c.request);
  const status = requireString(body.review_status, "review_status", 32);
  if (!["unreviewed", "reviewed", "rejected"].includes(status)) throw new HttpError(400, "invalid review_status");
  const result = await c.env.DB.prepare("UPDATE scenarios SET review_status = ?3, reviewed_by = ?4, reviewed_at = ?5, review_notes = ?6, updated_at = ?5 WHERE scenario_id = ?1 AND tenant_id = ?2")
    .bind(id, who.tenant, status, who.actor, nowIso(), optionalString(body.notes, 2000)).run();
  if (!result.meta.changes) throw new HttpError(404, "scenario not found");
  await audit(c.env, who, "scenario.review", id, { review_status: status });
  return json({ scenario_id: id, review_status: status });
}
