import { McapWriter, TempBuffer } from "@mcap/core";
import { LocationFix, Log } from "@foxglove/schemas/jsonschema";
import type { RunRow, Sample } from "./types";

/**
 * Run export formats for external tools:
 *   * GeoJSON  - kepler.gl, QGIS, Mapbox, deck.gl
 *   * CSV      - spreadsheets, pandas, PlotJuggler
 *   * MCAP     - Foxglove Studio, ROS 2 tooling (foxglove.LocationFix / foxglove.Log channels
 *                plus the full Atlas sample on /atlas/telemetry)
 */

export interface ExportEvent {
  event_id: string;
  t: number;
  frame?: number | null;
  event_class: string;
  severity: string;
  lat?: number | null;
  lon?: number | null;
  description?: string | null;
  rule_id?: string | null;
  data?: unknown;
  review_status?: string;
}

export const SAMPLE_COLUMNS: Array<keyof Sample> = [
  "t", "frame", "lat", "lon", "alt", "speed_mps", "heading_deg", "accel_x", "accel_y", "accel_z", "yaw_rate",
  "throttle", "brake", "steer", "grade_pct", "lead_distance_m", "lead_rel_speed_mps", "source",
];

export function runToGeoJson(run: RunRow, samples: Sample[], events: ExportEvent[]): Record<string, unknown> {
  const coordinates = samples.map((s) => (typeof s.alt === "number" ? [s.lon, s.lat, s.alt] : [s.lon, s.lat]));
  const features: Array<Record<string, unknown>> = [];
  if (coordinates.length) {
    features.push({
      type: "Feature",
      id: run.run_id,
      geometry: { type: "LineString", coordinates },
      properties: {
        kind: "route",
        run_id: run.run_id,
        source: run.source,
        vehicle_class: run.vehicle_class,
        scenario_id: run.scenario_id,
        route_id: run.route_id,
        started_at: run.started_at,
        duration_s: run.duration_s,
        distance_km: run.distance_km,
        quality_status: run.quality_status,
        sample_count: samples.length,
        timestamps: samples.map((s) => s.t),
        speed_mps: samples.map((s) => s.speed_mps ?? null),
      },
    });
  }
  for (const e of events) {
    if (e.lat == null || e.lon == null) continue;
    features.push({
      type: "Feature",
      id: e.event_id,
      geometry: { type: "Point", coordinates: [e.lon, e.lat] },
      properties: { kind: "event", run_id: run.run_id, t: e.t, frame: e.frame ?? null, event_class: e.event_class, severity: e.severity, rule_id: e.rule_id ?? null, description: e.description ?? "", review_status: e.review_status ?? null, data: e.data ?? null },
    });
  }
  return { type: "FeatureCollection", features, atlas: { run_id: run.run_id, tenant: run.tenant_id, generated_at: new Date().toISOString() } };
}

export function samplesToCsv(samples: Sample[]): string {
  const rows = [SAMPLE_COLUMNS.join(",")];
  for (const s of samples) {
    rows.push(SAMPLE_COLUMNS.map((c) => csvCell((s as unknown as Record<string, unknown>)[c])).join(","));
  }
  return rows.join("\n") + "\n";
}

function csvCell(value: unknown): string {
  if (value === null || value === undefined) return "";
  const text = typeof value === "number" ? String(value) : String(value);
  return /[",\n]/.test(text) ? '"' + text.replace(/"/g, '""') + '"' : text;
}

const ATLAS_SAMPLE_SCHEMA = {
  title: "atlas.TelemetrySample",
  type: "object",
  description: "One row of the Bhutan Atlas unified timeline (see PythonAPI/bhutan/bhutan_sim/telemetry.py).",
  properties: Object.fromEntries(SAMPLE_COLUMNS.map((c) => [c, { type: c === "source" ? "string" : "number" }])),
};

const LOG_LEVEL: Record<string, number> = { info: 2, warning: 3, critical: 4 };

function toTime(t: number): { sec: number; nsec: number } {
  const sec = Math.floor(t);
  return { sec, nsec: Math.round((t - sec) * 1e9) };
}

function nanos(t: number): bigint {
  return BigInt(Math.round(t * 1e6)) * 1000n;
}

export async function runToMcap(run: RunRow, samples: Sample[], events: ExportEvent[]): Promise<Uint8Array> {
  const buffer = new TempBuffer();
  const writer = new McapWriter({ writable: buffer, useStatistics: true, useChunks: true, useChunkIndex: true, useMessageIndex: true });
  await writer.start({ profile: "", library: "bhutan-atlas" });
  const encoder = new TextEncoder();
  const fixSchema = await writer.registerSchema({ name: "foxglove.LocationFix", encoding: "jsonschema", data: encoder.encode(JSON.stringify(LocationFix)) });
  const logSchema = await writer.registerSchema({ name: "foxglove.Log", encoding: "jsonschema", data: encoder.encode(JSON.stringify(Log)) });
  const sampleSchema = await writer.registerSchema({ name: "atlas.TelemetrySample", encoding: "jsonschema", data: encoder.encode(JSON.stringify(ATLAS_SAMPLE_SCHEMA)) });
  const fixChannel = await writer.registerChannel({ topic: "/gnss/fix", schemaId: fixSchema, messageEncoding: "json", metadata: new Map() });
  const sampleChannel = await writer.registerChannel({ topic: "/atlas/telemetry", schemaId: sampleSchema, messageEncoding: "json", metadata: new Map() });
  const logChannel = await writer.registerChannel({ topic: "/atlas/events", schemaId: logSchema, messageEncoding: "json", metadata: new Map() });
  await writer.addMetadata({
    name: "atlas.run",
    metadata: new Map<string, string>([
      ["run_id", run.run_id], ["tenant", run.tenant_id], ["source", run.source], ["vehicle_class", run.vehicle_class],
      ["scenario_id", run.scenario_id || ""], ["route_id", run.route_id || ""], ["quality_status", run.quality_status],
      ["distance_km", String(run.distance_km)], ["duration_s", String(run.duration_s)],
    ]),
  });
  let seq = 0;
  for (const s of samples) {
    const time = nanos(s.t);
    const fix = { timestamp: toTime(s.t), frame_id: "gnss", latitude: s.lat, longitude: s.lon, altitude: s.alt ?? 0, position_covariance: [0, 0, 0, 0, 0, 0, 0, 0, 0], position_covariance_type: 0 };
    await writer.addMessage({ channelId: fixChannel, sequence: seq, logTime: time, publishTime: time, data: encoder.encode(JSON.stringify(fix)) });
    await writer.addMessage({ channelId: sampleChannel, sequence: seq, logTime: time, publishTime: time, data: encoder.encode(JSON.stringify(s)) });
    seq += 1;
  }
  let logSeq = 0;
  for (const e of events) {
    const time = nanos(e.t);
    const log = { timestamp: toTime(e.t), level: LOG_LEVEL[e.severity] ?? 2, message: `${e.event_class}${e.rule_id ? " [" + e.rule_id + "]" : ""}: ${e.description || ""}`.trim(), name: e.rule_id || e.event_class, file: run.run_id, line: e.frame ?? 0 };
    await writer.addMessage({ channelId: logChannel, sequence: logSeq++, logTime: time, publishTime: time, data: encoder.encode(JSON.stringify(log)) });
  }
  await writer.end();
  return buffer.get();
}
