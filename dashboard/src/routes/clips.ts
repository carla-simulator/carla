import type { RouteContext } from "../router";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { HttpError, json, nowIso, optionalNumber, optionalString, readJson, requireId, sha256 } from "../util";

const REDACTION = new Set(["pending", "redacted", "failed", "not_required"]);

interface ClipRow {
  clip_id: string;
  tenant_id: string;
  run_id: string | null;
  r2_key: string | null;
  content_type: string | null;
  size_bytes: number;
  sha256: string | null;
  redaction_status: string;
  consent_ref: string | null;
  released: number;
}

export async function listClips(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const run = c.url.searchParams.get("run_id");
  const binds: unknown[] = [who.tenant];
  let sql = "SELECT * FROM clips WHERE tenant_id = ?1";
  if (run) { binds.push(run); sql += " AND run_id = ?2"; }
  sql += " ORDER BY created_at DESC LIMIT 500";
  const rows = await c.env.DB.prepare(sql).bind(...binds).all();
  return json({ clips: rows.results });
}

export async function registerClip(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<{ clip_id: string; run_id?: string; t_start?: number; t_end?: number; content_type?: string; consent_ref?: string; redaction_status?: string; notes?: string }>(c.request);
  const id = requireId(body.clip_id, "clip_id");
  const redaction = body.redaction_status || "pending";
  if (!REDACTION.has(redaction)) throw new HttpError(400, "invalid redaction_status");
  await c.env.DB.prepare(
    `INSERT INTO clips (clip_id, tenant_id, run_id, t_start, t_end, content_type, consent_ref, redaction_status, notes, updated_at)
     VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)
     ON CONFLICT(clip_id) DO UPDATE SET run_id = excluded.run_id, t_start = excluded.t_start, t_end = excluded.t_end, content_type = excluded.content_type,
       consent_ref = excluded.consent_ref, redaction_status = excluded.redaction_status, notes = excluded.notes, updated_at = excluded.updated_at
     WHERE clips.tenant_id = excluded.tenant_id`,
  ).bind(id, who.tenant, optionalString(body.run_id, 200), optionalNumber(body.t_start), optionalNumber(body.t_end), optionalString(body.content_type, 100) || "video/mp4",
    optionalString(body.consent_ref, 200), redaction, optionalString(body.notes, 2000), nowIso()).run();
  await audit(c.env, who, "clip.register", id, { run_id: body.run_id, consent_ref: body.consent_ref });
  return json({ clip_id: id }, 201);
}

async function getClip(c: RouteContext, tenant: string, id: string): Promise<ClipRow> {
  const row = await c.env.DB.prepare("SELECT * FROM clips WHERE clip_id = ?1 AND tenant_id = ?2").bind(id, tenant).first<ClipRow>();
  if (!row) throw new HttpError(404, "clip not found");
  return row;
}

export async function putClipObject(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const id = requireId(c.params.id, "clip_id");
  const clip = await getClip(c, who.tenant, id);
  const max = Number(c.env.MAX_CLIP_BYTES || 524288000);
  const length = Number(c.request.headers.get("content-length") || 0);
  if (length > max) throw new HttpError(413, `clip exceeds ${max} bytes`);
  const bytes = await c.request.arrayBuffer();
  if (bytes.byteLength === 0) throw new HttpError(400, "empty body");
  if (bytes.byteLength > max) throw new HttpError(413, `clip exceeds ${max} bytes`);
  const digest = await sha256(bytes);
  const contentType = c.request.headers.get("content-type") || clip.content_type || "application/octet-stream";
  const key = `${who.tenant}/clips/${id}`;
  await c.env.STORAGE.put(key, bytes, { httpMetadata: { contentType }, customMetadata: { sha256: digest, clip_id: id } });
  await c.env.DB.prepare("UPDATE clips SET r2_key = ?3, content_type = ?4, size_bytes = ?5, sha256 = ?6, updated_at = ?7 WHERE clip_id = ?1 AND tenant_id = ?2")
    .bind(id, who.tenant, key, contentType, bytes.byteLength, digest, nowIso()).run();
  await audit(c.env, who, "clip.upload", id, { size_bytes: bytes.byteLength, sha256: digest });
  return json({ clip_id: id, r2_key: key, size_bytes: bytes.byteLength, sha256: digest }, 201);
}

export async function getClipObject(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const id = requireId(c.params.id, "clip_id");
  const clip = await getClip(c, who.tenant, id);
  if (!clip.r2_key) throw new HttpError(404, "clip has no object");
  // Readers only see clips that have cleared governance; writers and admins see everything in their tenant.
  const cleared = clip.released && ["redacted", "not_required"].includes(clip.redaction_status) && clip.consent_ref;
  if (who.role === "reader" && !cleared) throw new HttpError(403, "clip is not released");
  const object = await c.env.STORAGE.get(clip.r2_key);
  if (!object) throw new HttpError(404, "object missing from storage");
  await audit(c.env, who, "clip.read", id, null);
  const headers = new Headers();
  object.writeHttpMetadata(headers);
  headers.set("etag", object.httpEtag);
  headers.set("cache-control", "private, no-store");
  return new Response(object.body, { headers });
}

export async function updateClipGovernance(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const id = requireId(c.params.id, "clip_id");
  await getClip(c, who.tenant, id);
  const body = await readJson<{ redaction_status?: string; consent_ref?: string; released?: boolean }>(c.request);
  if (body.redaction_status && !REDACTION.has(body.redaction_status)) throw new HttpError(400, "invalid redaction_status");
  const current = await getClip(c, who.tenant, id);
  const redaction = body.redaction_status || current.redaction_status;
  const consent = body.consent_ref !== undefined ? optionalString(body.consent_ref, 200) : current.consent_ref;
  let released = body.released === undefined ? Boolean(current.released) : Boolean(body.released);
  if (released && !(["redacted", "not_required"].includes(redaction) && consent)) {
    throw new HttpError(409, "a clip can only be released once redaction is complete and a consent reference is recorded");
  }
  await c.env.DB.prepare("UPDATE clips SET redaction_status = ?3, consent_ref = ?4, released = ?5, updated_at = ?6 WHERE clip_id = ?1 AND tenant_id = ?2")
    .bind(id, who.tenant, redaction, consent, released ? 1 : 0, nowIso()).run();
  await audit(c.env, who, "clip.governance", id, { redaction_status: redaction, consent_ref: consent, released });
  return json({ clip_id: id, redaction_status: redaction, consent_ref: consent, released });
}
