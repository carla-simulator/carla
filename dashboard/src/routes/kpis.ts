import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { computeKpis, snapshotKpis } from "../kpi";
import { renderPrometheus } from "../metrics";
import { openApiDocument } from "../openapi";
import { json, parseJsonColumn } from "../util";

export async function getKpis(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  return json(await computeKpis(c.env, who.tenant));
}

/** Prometheus / Grafana scrape target. */
export async function getMetrics(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const [report, fleet, scores] = await Promise.all([
    computeKpis(c.env, who.tenant),
    c.env.DB.prepare("SELECT COUNT(*) AS devices, SUM(CASE WHEN last_t >= ?2 THEN 1 ELSE 0 END) AS online FROM fleet_devices WHERE tenant_id = ?1").bind(who.tenant, Date.now() / 1000 - 300).first<{ devices: number; online: number | null }>(),
    c.env.DB.prepare("SELECT AVG(driving_score) AS avg_score, COUNT(driving_score) AS scored FROM runs WHERE tenant_id = ?1 AND driving_score IS NOT NULL").bind(who.tenant).first<{ avg_score: number | null; scored: number }>(),
  ]);
  const extra: Record<string, number> = { atlas_fleet_devices: fleet?.devices || 0, atlas_fleet_devices_online: fleet?.online || 0, atlas_runs_scored: scores?.scored || 0 };
  if (scores?.avg_score !== null && scores?.avg_score !== undefined) extra.atlas_driving_score_avg = Math.round(scores.avg_score * 100) / 100;
  return new Response(renderPrometheus(report, extra), { headers: { "content-type": "text/plain; version=0.0.4; charset=utf-8", "cache-control": "no-store" } });
}

export function getOpenApi(c: RouteContext): Response {
  return json(openApiDocument(c.url.origin, c.env.APP_NAME), 200, { "cache-control": "public, max-age=300" });
}

export async function getKpiHistory(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const limit = Math.min(365, Math.max(1, Number(c.url.searchParams.get("limit") || 90)));
  const rows = await c.env.DB.prepare("SELECT computed_at, payload FROM kpi_snapshots WHERE tenant_id = ?1 ORDER BY computed_at DESC LIMIT ?2").bind(who.tenant, limit).all<{ computed_at: string; payload: string }>();
  return json({ snapshots: rows.results.map((r) => ({ computed_at: r.computed_at, ...parseJsonColumn<Record<string, unknown>>(r.payload, {}) })).reverse() });
}

export async function postKpiSnapshot(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  return json(await snapshotKpis(c.env, who.tenant), 201);
}

export async function listAudit(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const limit = Math.min(500, Math.max(1, Number(c.url.searchParams.get("limit") || 100)));
  const rows = await c.env.DB.prepare("SELECT id, actor, action, target, detail, created_at FROM audit_log WHERE tenant_id = ?1 ORDER BY id DESC LIMIT ?2").bind(who.tenant, limit).all();
  return json({ audit: rows.results.map((r) => ({ ...r, detail: parseJsonColumn(r.detail, null) })) });
}
