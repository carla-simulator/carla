import type { RouteContext } from "../router";
import type { Sample } from "../types";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { HttpError, haversineKm, json, nowIso, optionalNumber, optionalString, parseJsonColumn, readJson, requireId } from "../util";
import { storeTelemetryChunk } from "./runs";

/**
 * Live fleet ingestion.
 *
 * Three producers feed the same `fleet_positions` table:
 *   * Traccar event/position forwarding (JSON body with `position` and `device`),
 *     configured in traccar.xml with `forward.enable`, `forward.url` and
 *     `forward.header` = "Authorization: Bearer <writer token>".
 *   * OsmAnd protocol (Traccar Client, OsmAnd, GPSLogger): GET or POST with
 *     query parameters id, lat, lon, timestamp, speed, bearing, altitude, hdop.
 *     Put the writer token in the `token` query parameter.
 *   * Generic JSON: POST /api/fleet/positions {positions: [...]} from edge loggers.
 *
 * A device track can later be materialised into a run so it counts toward
 * the real-world coverage KPI and gets quality gates, events and evidence.
 */

const KNOTS_TO_MPS = 0.514444;
const MAX_POSITIONS_PER_REQUEST = 2000;

export interface PositionIn {
  device_id: string;
  t: number;
  lat: number;
  lon: number;
  alt?: number | null;
  speed_mps?: number | null;
  heading_deg?: number | null;
  accuracy_m?: number | null;
  attributes?: Record<string, unknown> | null;
  name?: string | null;
  vehicle_class?: string | null;
}

interface PositionRow {
  t: number;
  lat: number;
  lon: number;
  alt: number | null;
  speed_mps: number | null;
  heading_deg: number | null;
  accuracy_m: number | null;
  source: string;
  attributes: string | null;
}

function parseTime(value: unknown): number | null {
  if (typeof value === "number" && Number.isFinite(value)) return value > 1e12 ? value / 1000 : value;
  if (typeof value === "string" && value) {
    const n = Number(value);
    if (Number.isFinite(n)) return n > 1e12 ? n / 1000 : n;
    const ms = Date.parse(value);
    if (Number.isFinite(ms)) return ms / 1000;
  }
  return null;
}

/** Traccar "forward" JSON: {"position": {...}, "device": {...}, "event": {...}?}. Also accepts a bare position. */
export function fromTraccar(body: Record<string, unknown>): PositionIn {
  const position = (body.position && typeof body.position === "object" ? body.position : body) as Record<string, unknown>;
  const device = (body.device && typeof body.device === "object" ? body.device : {}) as Record<string, unknown>;
  const deviceId = String(device.uniqueId || device.id || position.deviceId || position.device_id || "");
  const t = parseTime(position.fixTime) ?? parseTime(position.deviceTime) ?? parseTime(position.serverTime);
  if (!deviceId) throw new HttpError(400, "Traccar payload has no device id");
  if (t === null) throw new HttpError(400, "Traccar payload has no fixTime/deviceTime");
  const lat = optionalNumber(position.latitude);
  const lon = optionalNumber(position.longitude);
  if (lat === null || lon === null) throw new HttpError(400, "Traccar payload has no latitude/longitude");
  const attributes = (position.attributes && typeof position.attributes === "object" ? position.attributes : {}) as Record<string, unknown>;
  const speedKnots = optionalNumber(position.speed);
  return {
    device_id: deviceId,
    name: optionalString(device.name, 200),
    vehicle_class: optionalString(device.category, 32),
    t, lat, lon,
    alt: optionalNumber(position.altitude),
    speed_mps: speedKnots === null ? null : speedKnots * KNOTS_TO_MPS,
    heading_deg: optionalNumber(position.course),
    accuracy_m: optionalNumber(position.accuracy),
    attributes: { ...attributes, protocol: position.protocol, event: body.event && typeof body.event === "object" ? (body.event as Record<string, unknown>).type : undefined },
  };
}

/** OsmAnd protocol query string used by Traccar Client and GPSLogger. */
export function fromOsmAnd(params: URLSearchParams): PositionIn {
  const deviceId = params.get("id") || params.get("deviceid") || "";
  if (!deviceId) throw new HttpError(400, "id is required");
  const t = parseTime(params.get("timestamp")) ?? Date.now() / 1000;
  const lat = optionalNumber(params.get("lat"));
  const lon = optionalNumber(params.get("lon"));
  if (lat === null || lon === null) throw new HttpError(400, "lat and lon are required");
  const speedKnots = optionalNumber(params.get("speed"));
  const attributes: Record<string, unknown> = {};
  for (const key of ["batt", "hdop", "charge", "ignition", "driverUniqueId"]) {
    const v = params.get(key);
    if (v !== null) attributes[key] = optionalNumber(v) ?? v;
  }
  return {
    device_id: deviceId, t, lat, lon,
    alt: optionalNumber(params.get("altitude")),
    speed_mps: speedKnots === null ? null : speedKnots * KNOTS_TO_MPS,
    heading_deg: optionalNumber(params.get("bearing")),
    accuracy_m: optionalNumber(params.get("accuracy")),
    attributes,
  };
}

async function storePositions(c: RouteContext, tenant: string, positions: PositionIn[], source: string): Promise<number> {
  if (positions.length === 0) throw new HttpError(400, "no positions");
  if (positions.length > MAX_POSITIONS_PER_REQUEST) throw new HttpError(413, `at most ${MAX_POSITIONS_PER_REQUEST} positions per request`);
  const statements: D1PreparedStatement[] = [];
  const latest = new Map<string, PositionIn>();
  for (const p of positions) {
    const deviceId = requireId(p.device_id, "device_id");
    if (typeof p.t !== "number" || typeof p.lat !== "number" || typeof p.lon !== "number") throw new HttpError(400, "every position needs numeric t, lat and lon");
    if (Math.abs(p.lat) > 90 || Math.abs(p.lon) > 180) throw new HttpError(400, "lat/lon out of range");
    statements.push(
      c.env.DB.prepare(
        `INSERT INTO fleet_positions (tenant_id, device_id, t, lat, lon, alt, speed_mps, heading_deg, accuracy_m, source, attributes)
         VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)
         ON CONFLICT(tenant_id, device_id, t) DO UPDATE SET lat = excluded.lat, lon = excluded.lon, alt = excluded.alt, speed_mps = excluded.speed_mps,
           heading_deg = excluded.heading_deg, accuracy_m = excluded.accuracy_m, attributes = excluded.attributes`,
      ).bind(tenant, deviceId, p.t, p.lat, p.lon, optionalNumber(p.alt), optionalNumber(p.speed_mps), optionalNumber(p.heading_deg), optionalNumber(p.accuracy_m), source,
        p.attributes ? JSON.stringify(p.attributes).slice(0, 4000) : null),
    );
    const prev = latest.get(deviceId);
    if (!prev || prev.t <= p.t) latest.set(deviceId, { ...p, device_id: deviceId });
  }
  for (const [deviceId, p] of latest) {
    statements.push(
      c.env.DB.prepare(
        `INSERT INTO fleet_devices (tenant_id, device_id, name, vehicle_class, source, last_t, last_lat, last_lon, last_alt, last_speed_mps, last_heading_deg, position_count, attributes, updated_at)
         VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, 0, ?12, ?13)
         ON CONFLICT(tenant_id, device_id) DO UPDATE SET
           name = COALESCE(excluded.name, fleet_devices.name), vehicle_class = COALESCE(?4, fleet_devices.vehicle_class), source = excluded.source,
           last_t = CASE WHEN excluded.last_t >= COALESCE(fleet_devices.last_t, 0) THEN excluded.last_t ELSE fleet_devices.last_t END,
           last_lat = CASE WHEN excluded.last_t >= COALESCE(fleet_devices.last_t, 0) THEN excluded.last_lat ELSE fleet_devices.last_lat END,
           last_lon = CASE WHEN excluded.last_t >= COALESCE(fleet_devices.last_t, 0) THEN excluded.last_lon ELSE fleet_devices.last_lon END,
           last_alt = CASE WHEN excluded.last_t >= COALESCE(fleet_devices.last_t, 0) THEN excluded.last_alt ELSE fleet_devices.last_alt END,
           last_speed_mps = CASE WHEN excluded.last_t >= COALESCE(fleet_devices.last_t, 0) THEN excluded.last_speed_mps ELSE fleet_devices.last_speed_mps END,
           last_heading_deg = CASE WHEN excluded.last_t >= COALESCE(fleet_devices.last_t, 0) THEN excluded.last_heading_deg ELSE fleet_devices.last_heading_deg END,
           attributes = COALESCE(excluded.attributes, fleet_devices.attributes), updated_at = excluded.updated_at`,
      ).bind(tenant, deviceId, optionalString(p.name, 200), p.vehicle_class ? optionalString(p.vehicle_class, 32) : null, source, p.t, p.lat, p.lon,
        optionalNumber(p.alt), optionalNumber(p.speed_mps), optionalNumber(p.heading_deg), p.attributes ? JSON.stringify(p.attributes).slice(0, 4000) : null, nowIso()),
    );
    statements.push(c.env.DB.prepare("UPDATE fleet_devices SET position_count = (SELECT COUNT(*) FROM fleet_positions WHERE tenant_id = ?1 AND device_id = ?2) WHERE tenant_id = ?1 AND device_id = ?2").bind(tenant, deviceId));
  }
  await c.env.DB.batch(statements);
  return positions.length;
}

export async function ingestTraccar(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<Record<string, unknown> | Record<string, unknown>[]>(c.request, 5 * 1024 * 1024);
  const items = Array.isArray(body) ? body : [body];
  const positions = items.map(fromTraccar);
  const stored = await storePositions(c, who.tenant, positions, "traccar");
  return json({ stored, devices: [...new Set(positions.map((p) => p.device_id))] }, 201);
}

export async function ingestOsmAnd(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  let params = c.url.searchParams;
  if (c.request.method === "POST") {
    const text = await c.request.text();
    if (text.trim()) params = new URLSearchParams(text);
  }
  const position = fromOsmAnd(params);
  await storePositions(c, who.tenant, [position], "osmand");
  return new Response("OK", { status: 200, headers: { "content-type": "text/plain" } });
}

export async function postPositions(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const body = await readJson<{ positions: PositionIn[] }>(c.request, 20 * 1024 * 1024);
  const positions = Array.isArray(body.positions) ? body.positions : [];
  const stored = await storePositions(c, who.tenant, positions, "generic");
  return json({ stored }, 201);
}

export async function listFleet(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const rows = await c.env.DB.prepare("SELECT * FROM fleet_devices WHERE tenant_id = ?1 ORDER BY last_t DESC LIMIT 500").bind(who.tenant).all<Record<string, unknown>>();
  const now = Date.now() / 1000;
  return json({
    now,
    devices: rows.results.map((d) => ({ ...d, attributes: parseJsonColumn(d.attributes, {}), age_s: typeof d.last_t === "number" ? Math.max(0, now - d.last_t) : null, online: typeof d.last_t === "number" && now - d.last_t < 300 })),
  });
}

async function loadTrack(c: RouteContext, tenant: string, deviceId: string, since: number | null, until: number | null, limit: number): Promise<PositionRow[]> {
  const binds: unknown[] = [tenant, deviceId];
  let sql = "SELECT t, lat, lon, alt, speed_mps, heading_deg, accuracy_m, source, attributes FROM fleet_positions WHERE tenant_id = ?1 AND device_id = ?2";
  if (since !== null) { binds.push(since); sql += ` AND t >= ?${binds.length}`; }
  if (until !== null) { binds.push(until); sql += ` AND t <= ?${binds.length}`; }
  binds.push(limit);
  sql += ` ORDER BY t DESC LIMIT ?${binds.length}`;
  const rows = await c.env.DB.prepare(sql).bind(...binds).all<PositionRow>();
  return rows.results.reverse();
}

export async function deviceTrack(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const deviceId = requireId(c.params.id, "device_id");
  const since = optionalNumber(c.url.searchParams.get("since"));
  const until = optionalNumber(c.url.searchParams.get("until"));
  const limit = Math.min(20000, Math.max(1, Number(c.url.searchParams.get("limit") || 5000)));
  const positions = await loadTrack(c, who.tenant, deviceId, since, until, limit);
  return json({ device_id: deviceId, positions: positions.map((p) => ({ ...p, attributes: parseJsonColumn(p.attributes, null) })) });
}

/** Turn fleet positions into unified-timeline samples (speed and heading derived when the device did not report them). */
export function positionsToSamples(positions: PositionRow[], vehicleSource = "vehicle"): Sample[] {
  const samples: Sample[] = [];
  let prev: PositionRow | null = null;
  let prevSpeed = 0;
  positions.forEach((p, i) => {
    let speed = p.speed_mps ?? 0;
    let heading = p.heading_deg ?? 0;
    if (prev) {
      const dt = p.t - prev.t;
      const km = haversineKm(prev.lat, prev.lon, p.lat, p.lon);
      if (p.speed_mps === null && dt > 0) speed = (km * 1000) / dt;
      if (p.heading_deg === null) heading = (Math.atan2(p.lon - prev.lon, p.lat - prev.lat) * 180 / Math.PI + 360) % 360;
      const accel = dt > 0 ? (speed - prevSpeed) / dt : 0;
      const grade = prev.alt !== null && p.alt !== null && km > 0 ? ((p.alt - prev.alt) / (km * 1000)) * 100 : 0;
      samples.push({ t: p.t, frame: i, lat: p.lat, lon: p.lon, alt: p.alt ?? 0, speed_mps: speed, heading_deg: heading, accel_x: accel, accel_y: 0, accel_z: 0, yaw_rate: 0, grade_pct: grade, source: vehicleSource });
    } else {
      samples.push({ t: p.t, frame: i, lat: p.lat, lon: p.lon, alt: p.alt ?? 0, speed_mps: speed, heading_deg: heading, accel_x: 0, accel_y: 0, accel_z: 0, yaw_rate: 0, grade_pct: 0, source: vehicleSource });
    }
    prev = p;
    prevSpeed = speed;
  });
  return samples;
}

/** Create a run from a device's track so it enters the catalog, KPIs and evidence flow. */
export async function materializeTrack(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "writer");
  const deviceId = requireId(c.params.id, "device_id");
  const body = await readJson<{ since?: number; until?: number; run_id?: string; route_id?: string; consent_ref?: string; vehicle_class?: string; notes?: string }>(c.request);
  const since = optionalNumber(body.since);
  const until = optionalNumber(body.until);
  const positions = await loadTrack(c, who.tenant, deviceId, since, until, 20000);
  if (positions.length < 2) throw new HttpError(400, "not enough positions in the requested window");
  const device = await c.env.DB.prepare("SELECT name, vehicle_class FROM fleet_devices WHERE tenant_id = ?1 AND device_id = ?2").bind(who.tenant, deviceId).first<{ name: string | null; vehicle_class: string }>();
  const runId = body.run_id ? requireId(body.run_id, "run_id") : `fleet-${deviceId}-${new Date(positions[0].t * 1000).toISOString().slice(0, 16).replace(/[-:T]/g, "")}`;
  const samples = positionsToSamples(positions);
  const vehicleClass = optionalString(body.vehicle_class, 32) || device?.vehicle_class || "truck";
  await c.env.DB.prepare(
    `INSERT INTO runs (run_id, tenant_id, source, vehicle_class, route_id, map_name, odd_zone, started_at, ended_at, has_gnss, privacy_status, consent_ref, notes, updated_at)
     VALUES (?1, ?2, 'vehicle', ?3, ?4, 'real-world', ?5, ?6, ?7, 1, 'not_required', ?8, ?9, ?10)
     ON CONFLICT(run_id) DO UPDATE SET started_at = excluded.started_at, ended_at = excluded.ended_at, consent_ref = COALESCE(excluded.consent_ref, runs.consent_ref), notes = excluded.notes, updated_at = excluded.updated_at
     WHERE runs.tenant_id = excluded.tenant_id`,
  ).bind(runId, who.tenant, vehicleClass, optionalString(body.route_id, 200) || `fleet:${deviceId}`, device?.name || deviceId, positions[0].t, positions[positions.length - 1].t,
    optionalString(body.consent_ref, 200), optionalString(body.notes, 4000) || `Materialised from fleet device ${deviceId} (${positions.length} positions, ${positions[0].source})`, nowIso()).run();
  const max = Number(c.env.MAX_TELEMETRY_SAMPLES_PER_CHUNK || 5000);
  let seq = 0;
  for (let i = 0; i < samples.length; i += max) {
    await storeTelemetryChunk(c.env, who.tenant, runId, seq++, samples.slice(i, i + max));
  }
  await audit(c.env, who, "fleet.materialize", runId, { device_id: deviceId, positions: positions.length, since, until });
  return json({ run_id: runId, device_id: deviceId, samples: samples.length, chunks: seq }, 201);
}
