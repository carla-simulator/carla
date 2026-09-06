import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { computeKpis, snapshotKpis } from "../kpi";
import { json, parseJsonColumn } from "../util";

export async function getKpis(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  return json(await computeKpis(c.env, who.tenant));
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
