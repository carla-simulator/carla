import type { RouteContext } from "../router";
import type { ChunkRow, Env, EventIn, QualityIn, RunIn, RunRow, Sample } from "../types";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { computeDrivingScore } from "../driving_score";
import { HttpError, haversineKm, json, nowIso, optionalNumber, optionalString, parseJsonColumn, readJson, requireId, requireString, sha256 } from "../util";

const ALLOWED_SEVERITY = new Set(["info", "warning", "critical"]);
const ALLOWED_PRIVACY = new Set(["unprocessed", "redacted", "not_required"]);

export async function listRuns(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const limit = Math.min(200, Math.max(1, Number(c.url.searchParams.get("limit") || 50)));
  const source = c.url.searchParams.get("source");
  const scenario = c.url.searchParams.get("scenario_id");
  const quality = c.url.searchParams.get("quality_status");
  const where: string[] = ["tenant_id = ?1"];
  const binds: unknown[] = [who.tenant];
  if (source) { binds.push(source); where.push(`source = ?${binds.length}`); }
  if (scenario) { binds.push(scenario); where.push(`scenario_id = ?${binds.length}`); }
  if (quality) { binds.push(quality); where.push(`quality_status = ?${binds.length}`); }
  binds.push(limit);
  const rows = await c.env.DB.prepare(
    `SELECT * FROM runs WHERE ${where.join(" AND ")} ORDER BY COALESCE(started_at, 0) DESC, created_at DESC LIMIT ?${binds.length}`,
  ).bind(...binds).all<RunRow>();
  return json({ runs: rows.results });
}

export async function createRun(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<RunIn>(c.request);
  const runId = requireId(body.run_id, "run_id");
  const source = requireString(body.source, "source", 32);
  if (!["sim", "vehicle"].includes(source)) throw new HttpError(400, "source must be sim or vehicle");
  const vehicleClass = requireString(body.vehicle_class, "vehicle_class", 32);
  const streams = body.streams || {};
  const privacy = body.privacy_status || (source === "sim" ? "not_required" : "unprocessed");
  if (!ALLOWED_PRIVACY.has(privacy)) throw new HttpError(400, "invalid privacy_status");
  await c.env.DB.prepare(
    `INSERT INTO runs (run_id, tenant_id, source, vehicle_class, route_id, scenario_id, scenario_hash, map_name, odd_zone, started_at, ended_at,
        has_video, has_gnss, has_imu, has_can, has_events, privacy_status, consent_ref, notes, updated_at)
     VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20)
     ON CONFLICT(run_id) DO UPDATE SET
        source = excluded.source, vehicle_class = excluded.vehicle_class, route_id = excluded.route_id, scenario_id = excluded.scenario_id,
        scenario_hash = excluded.scenario_hash, map_name = excluded.map_name, odd_zone = excluded.odd_zone, started_at = excluded.started_at,
        ended_at = COALESCE(excluded.ended_at, runs.ended_at), has_video = excluded.has_video, has_gnss = excluded.has_gnss, has_imu = excluded.has_imu,
        has_can = excluded.has_can, has_events = MAX(runs.has_events, excluded.has_events), privacy_status = excluded.privacy_status,
        consent_ref = excluded.consent_ref, notes = excluded.notes, updated_at = excluded.updated_at
     WHERE runs.tenant_id = excluded.tenant_id`,
  ).bind(
    runId, who.tenant, source, vehicleClass, optionalString(body.route_id), optionalString(body.scenario_id), optionalString(body.scenario_hash),
    optionalString(body.map_name), optionalString(body.odd_zone), optionalNumber(body.started_at), optionalNumber(body.ended_at),
    streams.video ? 1 : 0, streams.gnss ? 1 : 0, streams.imu ? 1 : 0, streams.can ? 1 : 0, streams.events ? 1 : 0,
    privacy, optionalString(body.consent_ref), optionalString(body.notes, 4000), nowIso(),
  ).run();
  await audit(c.env, who, "run.upsert", runId, { source, vehicle_class: vehicleClass, scenario_id: body.scenario_id });
  const row = await getRunRow(c, who.tenant, runId);
  return json({ run: row }, 201);
}

async function getRunRow(c: RouteContext, tenant: string, runId: string): Promise<RunRow> {
  const row = await c.env.DB.prepare("SELECT * FROM runs WHERE run_id = ?1 AND tenant_id = ?2").bind(runId, tenant).first<RunRow>();
  if (!row) throw new HttpError(404, "run not found");
  return row;
}

export async function getRun(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const runId = requireId(c.params.id, "run_id");
  const run = await getRunRow(c, who.tenant, runId);
  const [segments, chunks, eventSummary, evaluations] = await Promise.all([
    c.env.DB.prepare("SELECT idx, t_start, t_end, sample_count, passed, gates FROM segments WHERE run_id = ?1 ORDER BY idx").bind(runId).all(),
    c.env.DB.prepare("SELECT seq, sample_count, t_start, t_end, distance_km, sha256 FROM telemetry_chunks WHERE run_id = ?1 ORDER BY seq").bind(runId).all<ChunkRow>(),
    c.env.DB.prepare("SELECT event_class, severity, COUNT(*) AS n FROM events WHERE run_id = ?1 GROUP BY event_class, severity").bind(runId).all(),
    c.env.DB.prepare("SELECT evaluation_id, model_id, model_version, overall, reproducible, replay_verified, created_at FROM evaluations WHERE run_id = ?1 ORDER BY created_at DESC").bind(runId).all(),
  ]);
  return json({
    run: { ...run, infractions: parseJsonColumn(run.infractions, null) },
    segments: segments.results.map((s) => ({ ...s, gates: parseJsonColumn(s.gates, []) })),
    chunks: chunks.results,
    event_summary: eventSummary.results,
    evaluations: evaluations.results.map((e) => ({ ...e, overall: parseJsonColumn(e.overall, {}) })),
  });
}

function chunkStats(samples: Sample[]): { t_start: number | null; t_end: number | null; distance_km: number; bbox: number[] | null } {
  let tStart: number | null = null;
  let tEnd: number | null = null;
  let distance = 0;
  let minLat = Infinity, minLon = Infinity, maxLat = -Infinity, maxLon = -Infinity;
  let prev: Sample | null = null;
  for (const s of samples) {
    if (typeof s.t === "number") {
      tStart = tStart === null ? s.t : Math.min(tStart, s.t);
      tEnd = tEnd === null ? s.t : Math.max(tEnd, s.t);
    }
    if (typeof s.lat === "number" && typeof s.lon === "number") {
      minLat = Math.min(minLat, s.lat); maxLat = Math.max(maxLat, s.lat);
      minLon = Math.min(minLon, s.lon); maxLon = Math.max(maxLon, s.lon);
      if (prev) distance += haversineKm(prev.lat, prev.lon, s.lat, s.lon);
      prev = s;
    }
  }
  const bbox = Number.isFinite(minLat) ? [minLat, minLon, maxLat, maxLon] : null;
  return { t_start: tStart, t_end: tEnd, distance_km: distance, bbox };
}

/** Store one immutable telemetry chunk in R2 and index it in D1 (shared by uploads and fleet materialisation). */
export async function storeTelemetryChunk(env: Env, tenant: string, runId: string, seq: number, samples: Sample[]): Promise<Record<string, unknown>> {
  const jsonl = samples.map((s) => JSON.stringify(s)).join("\n") + "\n";
  const digest = await sha256(jsonl);
  const key = `${tenant}/runs/${runId}/telemetry/${String(seq).padStart(5, "0")}.jsonl`;
  await env.STORAGE.put(key, jsonl, { httpMetadata: { contentType: "application/x-ndjson" }, customMetadata: { sha256: digest, run_id: runId } });
  const stats = chunkStats(samples);
  const chunkId = `${runId}:${seq}`;
  const statements = [
    env.DB.prepare(
      `INSERT INTO telemetry_chunks (chunk_id, run_id, tenant_id, seq, r2_key, sample_count, t_start, t_end, distance_km, sha256)
       VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)
       ON CONFLICT(chunk_id) DO UPDATE SET r2_key = excluded.r2_key, sample_count = excluded.sample_count, t_start = excluded.t_start,
         t_end = excluded.t_end, distance_km = excluded.distance_km, sha256 = excluded.sha256`,
    ).bind(chunkId, runId, tenant, seq, key, samples.length, stats.t_start, stats.t_end, stats.distance_km, digest),
    env.DB.prepare(
      `UPDATE runs SET
         sample_count = (SELECT COALESCE(SUM(sample_count), 0) FROM telemetry_chunks WHERE run_id = ?1),
         distance_km = (SELECT COALESCE(SUM(distance_km), 0) FROM telemetry_chunks WHERE run_id = ?1),
         has_gnss = 1,
         bbox_min_lat = CASE WHEN ?2 IS NULL THEN bbox_min_lat ELSE MIN(COALESCE(bbox_min_lat, ?2), ?2) END,
         bbox_min_lon = CASE WHEN ?3 IS NULL THEN bbox_min_lon ELSE MIN(COALESCE(bbox_min_lon, ?3), ?3) END,
         bbox_max_lat = CASE WHEN ?4 IS NULL THEN bbox_max_lat ELSE MAX(COALESCE(bbox_max_lat, ?4), ?4) END,
         bbox_max_lon = CASE WHEN ?5 IS NULL THEN bbox_max_lon ELSE MAX(COALESCE(bbox_max_lon, ?5), ?5) END,
         updated_at = ?6
       WHERE run_id = ?1`,
    ).bind(runId, stats.bbox?.[0] ?? null, stats.bbox?.[1] ?? null, stats.bbox?.[2] ?? null, stats.bbox?.[3] ?? null, nowIso()),
  ];
  await env.DB.batch(statements);
  return { chunk_id: chunkId, r2_key: key, sample_count: samples.length, sha256: digest, ...stats };
}

export async function uploadTelemetry(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const runId = requireId(c.params.id, "run_id");
  await getRunRow(c, who.tenant, runId);
  const seq = Math.max(0, Number(c.url.searchParams.get("seq") || 0));
  const body = await readJson<{ samples: Sample[] }>(c.request);
  const samples = Array.isArray(body.samples) ? body.samples : [];
  const max = Number(c.env.MAX_TELEMETRY_SAMPLES_PER_CHUNK || 5000);
  if (samples.length === 0) throw new HttpError(400, "samples[] is empty");
  if (samples.length > max) throw new HttpError(413, `at most ${max} samples per chunk`);
  for (const s of samples) {
    if (typeof s.t !== "number" || typeof s.lat !== "number" || typeof s.lon !== "number") {
      throw new HttpError(400, "every sample needs numeric t, lat and lon");
    }
  }
  return json(await storeTelemetryChunk(c.env, who.tenant, runId, seq, samples), 201);
}

/** Merge every chunk of a run and downsample to at most maxPoints samples. */
export async function loadRunSamples(env: Env, tenant: string, runId: string, maxPoints: number): Promise<Sample[] & { total?: number; stride?: number }> {
  const chunks = await env.DB.prepare("SELECT * FROM telemetry_chunks WHERE run_id = ?1 AND tenant_id = ?2 ORDER BY seq").bind(runId, tenant).all<ChunkRow>();
  const total = chunks.results.reduce((n, ch) => n + ch.sample_count, 0);
  const stride = Math.max(1, Math.ceil(total / maxPoints));
  const samples: Sample[] & { total?: number; stride?: number } = [];
  let index = 0;
  for (const chunk of chunks.results) {
    const object = await env.STORAGE.get(chunk.r2_key);
    if (!object) continue;
    const text = await object.text();
    for (const line of text.split("\n")) {
      if (!line) continue;
      if (index++ % stride === 0) samples.push(JSON.parse(line) as Sample);
    }
  }
  samples.total = total;
  samples.stride = stride;
  return samples;
}

export async function getTelemetry(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const runId = requireId(c.params.id, "run_id");
  await getRunRow(c, who.tenant, runId);
  const maxPoints = Math.min(20000, Math.max(100, Number(c.url.searchParams.get("max") || 4000)));
  const samples = await loadRunSamples(c.env, who.tenant, runId, maxPoints);
  return json({ run_id: runId, total: samples.total, stride: samples.stride, samples: [...samples] }, 200, { "cache-control": "private, max-age=60" });
}

export async function uploadEvents(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const runId = requireId(c.params.id, "run_id");
  await getRunRow(c, who.tenant, runId);
  const body = await readJson<{ events: EventIn[] }>(c.request);
  const events = Array.isArray(body.events) ? body.events : [];
  if (events.length === 0) throw new HttpError(400, "events[] is empty");
  if (events.length > 5000) throw new HttpError(413, "at most 5000 events per request");
  const statements: D1PreparedStatement[] = [];
  for (const e of events) {
    if (typeof e.t !== "number") throw new HttpError(400, "every event needs a numeric t");
    const eventClass = requireString(e.event_class, "event_class", 64);
    const severity = e.severity || "info";
    if (!ALLOWED_SEVERITY.has(severity)) throw new HttpError(400, `invalid severity '${severity}'`);
    const id = e.event_id ? requireId(e.event_id, "event_id") : `${runId}:${e.frame ?? Math.round(e.t * 1000)}:${eventClass}:${e.rule_id || ""}`;
    statements.push(
      c.env.DB.prepare(
        `INSERT INTO events (event_id, run_id, tenant_id, t, frame, event_class, severity, lat, lon, description, rule_id, data, review_status)
         VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13)
         ON CONFLICT(event_id) DO UPDATE SET severity = excluded.severity, description = excluded.description, data = excluded.data`,
      ).bind(id, runId, who.tenant, e.t, optionalNumber(e.frame), eventClass, severity, optionalNumber(e.lat), optionalNumber(e.lon),
        optionalString(e.description, 1000), optionalString(e.rule_id, 32), e.data === undefined ? null : JSON.stringify(e.data).slice(0, 8000),
        e.rule_id ? "classified" : (e.review_status || "unreviewed")),
    );
  }
  statements.push(
    c.env.DB.prepare("UPDATE runs SET event_count = (SELECT COUNT(*) FROM events WHERE run_id = ?1), has_events = 1, updated_at = ?2 WHERE run_id = ?1").bind(runId, nowIso()),
  );
  await c.env.DB.batch(statements);
  return json({ run_id: runId, inserted: events.length }, 201);
}

export async function listEvents(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const runId = requireId(c.params.id, "run_id");
  await getRunRow(c, who.tenant, runId);
  const rows = await c.env.DB.prepare("SELECT * FROM events WHERE run_id = ?1 AND tenant_id = ?2 ORDER BY t LIMIT 5000").bind(runId, who.tenant).all();
  return json({ run_id: runId, events: rows.results.map((e) => ({ ...e, data: parseJsonColumn(e.data, null) })) });
}

export async function finishRun(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const runId = requireId(c.params.id, "run_id");
  const run = await getRunRow(c, who.tenant, runId);
  const body = await readJson<{ quality?: QualityIn; ended_at?: number }>(c.request);
  const q = body.quality || {};
  const endedAt = optionalNumber(body.ended_at) ?? run.ended_at ?? Date.now() / 1000;
  const chunkRange = await c.env.DB.prepare("SELECT MIN(t_start) AS t0, MAX(t_end) AS t1 FROM telemetry_chunks WHERE run_id = ?1").bind(runId).first<{ t0: number | null; t1: number | null }>();
  const duration = optionalNumber(q.duration_s) ?? (chunkRange?.t0 != null && chunkRange?.t1 != null ? chunkRange.t1 - chunkRange.t0 : (run.started_at ? endedAt - run.started_at : 0));
  const acceptance = optionalNumber(q.acceptance_rate);
  const passed = typeof q.passed === "boolean" ? q.passed : acceptance !== null ? acceptance >= 0.9 : null;
  const qualityStatus = passed === null ? "pending" : passed ? "accepted" : "rejected";
  const privacy = q.privacy_status && ALLOWED_PRIVACY.has(q.privacy_status) ? q.privacy_status : run.privacy_status;
  const streams = q.streams || {};
  // Leaderboard-style driving score from every event stored for the run; the planned
  // duration comes from the scenario template when the run executed one.
  const [eventRows, scenario] = await Promise.all([
    c.env.DB.prepare("SELECT event_class, severity, description, data FROM events WHERE run_id = ?1 AND tenant_id = ?2").bind(runId, who.tenant).all<{ event_class: string; severity: string; description: string | null; data: string | null }>(),
    run.scenario_id ? c.env.DB.prepare("SELECT params FROM scenarios WHERE scenario_id = ?1 AND tenant_id = ?2").bind(run.scenario_id, who.tenant).first<{ params: string }>() : Promise.resolve(null),
  ]);
  const planned = scenario ? optionalNumber(parseJsonColumn<Record<string, unknown>>(scenario.params, {}).duration_s) : null;
  const score = computeDrivingScore(eventRows.results.map((e) => ({ ...e, data: parseJsonColumn(e.data, null) })), duration, planned);
  const statements: D1PreparedStatement[] = [
    c.env.DB.prepare(
      `UPDATE runs SET ended_at = ?2, duration_s = ?3, distance_km = CASE WHEN ?4 IS NULL THEN distance_km ELSE ?4 END,
         quality_status = ?5, acceptance_rate = ?6, replay_complete = ?7, privacy_status = ?8,
         has_video = MAX(has_video, ?9), has_imu = MAX(has_imu, ?10), has_can = MAX(has_can, ?11), updated_at = ?12,
         driving_score = ?13, route_completion = ?14, infraction_penalty = ?15, infractions = ?16
       WHERE run_id = ?1`,
    ).bind(runId, endedAt, duration, optionalNumber(q.distance_km), qualityStatus, acceptance, q.replay_complete ? 1 : 0, privacy,
      streams.video ? 1 : 0, streams.imu ? 1 : 0, streams.can ? 1 : 0, nowIso(),
      score.driving_score, score.route_completion, score.infraction_penalty, JSON.stringify({ basis: score.route_completion_basis, infractions: score.infractions })),
    c.env.DB.prepare("DELETE FROM segments WHERE run_id = ?1").bind(runId),
  ];
  for (const seg of q.segments || []) {
    statements.push(
      c.env.DB.prepare("INSERT INTO segments (segment_id, run_id, tenant_id, idx, t_start, t_end, sample_count, passed, gates) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)")
        .bind(`${runId}:${seg.index}`, runId, who.tenant, seg.index, optionalNumber(seg.t_start), optionalNumber(seg.t_end), Number(seg.sample_count || 0), seg.passed ? 1 : 0, JSON.stringify(seg.gates ?? []).slice(0, 8000)),
    );
  }
  await c.env.DB.batch(statements);
  await audit(c.env, who, "run.finish", runId, { quality_status: qualityStatus, acceptance_rate: acceptance, segments: (q.segments || []).length, driving_score: score.driving_score });
  return json({ run: await getRunRow(c, who.tenant, runId), driving_score: score });
}

export async function reviewEvent(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const eventId = requireString(c.params.id, "event_id", 400);
  const body = await readJson<{ review_status: string; notes?: string }>(c.request);
  const status = requireString(body.review_status, "review_status", 32);
  if (!["unreviewed", "classified", "reviewed"].includes(status)) throw new HttpError(400, "invalid review_status");
  const result = await c.env.DB.prepare("UPDATE events SET review_status = ?3, reviewed_by = ?4, reviewed_at = ?5, review_notes = ?6 WHERE event_id = ?1 AND tenant_id = ?2")
    .bind(eventId, who.tenant, status, who.actor, nowIso(), optionalString(body.notes, 2000)).run();
  if (!result.meta.changes) throw new HttpError(404, "event not found");
  await audit(c.env, who, "event.review", eventId, { review_status: status });
  return json({ event_id: eventId, review_status: status });
}

export async function listCriticalEvents(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const status = c.url.searchParams.get("review_status");
  const binds: unknown[] = [who.tenant];
  let sql = "SELECT * FROM events WHERE tenant_id = ?1 AND severity = 'critical'";
  if (status) { binds.push(status); sql += ` AND review_status = ?${binds.length}`; }
  sql += " ORDER BY t DESC LIMIT 500";
  const rows = await c.env.DB.prepare(sql).bind(...binds).all();
  return json({ events: rows.results.map((e) => ({ ...e, data: parseJsonColumn(e.data, null) })) });
}
