/**
 * OpenAPI 3.1 description of the Atlas API, served at /api/openapi.json so
 * partners can generate clients or browse it in Swagger UI / Redoc.
 * Keep the paths in sync with src/index.ts.
 */

const bearer = [{ bearerAuth: [] }];

function op(summary: string, role: "reader" | "writer" | "admin" | "none", extra: Record<string, unknown> = {}): Record<string, unknown> {
  return { summary, description: role === "none" ? "No authentication." : `Requires the ${role} role.`, security: role === "none" ? [] : bearer, responses: { "200": { description: "OK" }, "401": { description: "Missing or invalid token" }, "403": { description: "Insufficient role" } }, ...extra };
}

const jsonBody = (schema: Record<string, unknown>) => ({ required: true, content: { "application/json": { schema } } });
const idParam = (name: string) => ({ name, in: "path", required: true, schema: { type: "string" } });
const q = (name: string, schema: Record<string, unknown> = { type: "string" }) => ({ name, in: "query", required: false, schema });

export function openApiDocument(baseUrl: string, appName: string): Record<string, unknown> {
  return {
    openapi: "3.1.0",
    info: {
      title: `${appName} API`,
      version: "0.2.0",
      description: "Operations and evidence API for the Bhutan mobility-data pilot: runs, telemetry, events, scenarios, evaluations, clips, fleet positions, KPIs, exports and metrics.",
    },
    servers: [{ url: baseUrl }],
    components: {
      securitySchemes: { bearerAuth: { type: "http", scheme: "bearer", description: "API_TOKENS entry; also accepted as ?token= for tools that cannot set headers." } },
      schemas: {
        Sample: { type: "object", required: ["t", "frame", "lat", "lon"], properties: { t: { type: "number" }, frame: { type: "integer" }, lat: { type: "number" }, lon: { type: "number" }, alt: { type: "number" }, speed_mps: { type: "number" }, heading_deg: { type: "number" }, accel_x: { type: "number" }, accel_y: { type: "number" }, accel_z: { type: "number" }, yaw_rate: { type: "number" }, throttle: { type: "number" }, brake: { type: "number" }, steer: { type: "number" }, grade_pct: { type: "number" }, lead_distance_m: { type: ["number", "null"] }, lead_rel_speed_mps: { type: ["number", "null"] }, source: { type: "string", enum: ["sim", "vehicle"] } } },
        Event: { type: "object", required: ["t", "event_class"], properties: { event_id: { type: "string" }, t: { type: "number" }, frame: { type: "integer" }, event_class: { type: "string" }, severity: { type: "string", enum: ["info", "warning", "critical"] }, lat: { type: "number" }, lon: { type: "number" }, description: { type: "string" }, rule_id: { type: "string" }, data: { type: "object" }, review_status: { type: "string", enum: ["unreviewed", "classified", "reviewed"] } } },
        Run: { type: "object", required: ["run_id", "source", "vehicle_class"], properties: { run_id: { type: "string" }, source: { type: "string", enum: ["sim", "vehicle"] }, vehicle_class: { type: "string" }, route_id: { type: "string" }, scenario_id: { type: "string" }, scenario_hash: { type: "string" }, map_name: { type: "string" }, odd_zone: { type: "string" }, started_at: { type: "number" }, ended_at: { type: "number" }, streams: { type: "object", additionalProperties: { type: "boolean" } }, consent_ref: { type: "string" }, notes: { type: "string" }, privacy_status: { type: "string", enum: ["unprocessed", "redacted", "not_required"] } } },
        Quality: { type: "object", properties: { duration_s: { type: "number" }, distance_km: { type: "number" }, acceptance_rate: { type: "number" }, replay_complete: { type: "boolean" }, passed: { type: "boolean" }, privacy_status: { type: "string" }, streams: { type: "object" }, segments: { type: "array", items: { type: "object" } } } },
        DrivingScore: { type: "object", properties: { driving_score: { type: "number" }, route_completion: { type: "number" }, infraction_penalty: { type: "number" }, route_completion_basis: { type: "string" }, infractions: { type: "array", items: { type: "object", properties: { event_class: { type: "string" }, kind: { type: "string" }, count: { type: "integer" }, penalty: { type: "number" } } } } } },
        Position: { type: "object", required: ["device_id", "t", "lat", "lon"], properties: { device_id: { type: "string" }, t: { type: "number" }, lat: { type: "number" }, lon: { type: "number" }, alt: { type: "number" }, speed_mps: { type: "number" }, heading_deg: { type: "number" }, accuracy_m: { type: "number" }, attributes: { type: "object" }, name: { type: "string" }, vehicle_class: { type: "string" } } },
        Scenario: { type: "object", required: ["id", "family", "content_hash", "params"], properties: { id: { type: "string" }, family: { type: "string" }, group: { type: "string" }, name: { type: "string" }, description: { type: "string" }, tags: { type: "array", items: { type: "string" } }, params: { type: "object" }, actors: { type: "array", items: { type: "object" } }, expected_events: { type: "array", items: { type: "string" } }, seed: { type: "integer" }, version: { type: "string" }, content_hash: { type: "string" } } },
        Evaluation: { type: "object", required: ["evaluation_id", "model_id", "overall"], properties: { evaluation_id: { type: "string" }, model_id: { type: "string" }, model_version: { type: "string" }, run_id: { type: "string" }, scenario_id: { type: "string" }, conditions: { type: "object" }, overall: { $ref: "#/components/schemas/Counts" }, by_class: { type: "object" }, by_condition: { type: "object" }, failure_clusters: { type: "array", items: { type: "object" } }, frames_evaluated: { type: "integer" }, inputs: { type: "object" }, reproducible: { type: "boolean" } } },
        Counts: { type: "object", properties: { tp: { type: "integer" }, fp: { type: "integer" }, fn: { type: "integer" }, precision: { type: "number" }, recall: { type: "number" }, f1: { type: "number" } } },
      },
    },
    paths: {
      "/api/health": { get: op("Health check", "none") },
      "/api/openapi.json": { get: op("This document", "none") },
      "/api/metrics": { get: op("Prometheus text exposition of KPIs, edge-case rates and perception metrics", "reader") },
      "/api/kpis": { get: op("Live KPI report", "reader") },
      "/api/kpis/history": { get: op("Nightly KPI snapshots", "reader", { parameters: [q("limit", { type: "integer" })] }) },
      "/api/kpis/snapshot": { post: op("Force a KPI snapshot", "writer") },
      "/api/budget/estimate": { get: op("Synthetic-data program budget: low/mid/high storage, GPU-hours and marketplace cost for N scenes, plus a coverage checklist of the scenario library and runs", "reader", { parameters: [q("scenes", { type: "integer", default: 100000 }), q("clip_seconds", { type: "number", default: 10 }), q("fps", { type: "number", default: 20 }), q("cameras", { type: "integer", default: 1 }), q("width", { type: "integer", default: 1280 }), q("height", { type: "integer", default: 720 }), q("workload", { type: "string", enum: ["finetune", "medium", "scratch"] }), q("gpu", { type: "string", enum: ["a100_80gb", "h100_80gb", "l40s_48gb", "rtx4090_24gb"] }), q("price_per_hour", { type: "number", description: "Override the reference USD per GPU-hour with a live quote" }), q("interruptible", { type: "boolean" }), q("parallel_gpus", { type: "integer", default: 8 }), q("storage_price_gb_month", { type: "number", default: 0.15 })] }) },
      "/api/runs": { get: op("List runs", "reader", { parameters: [q("source"), q("scenario_id"), q("quality_status"), q("limit", { type: "integer" })] }), post: op("Upsert a run manifest", "writer", { requestBody: jsonBody({ $ref: "#/components/schemas/Run" }) }) },
      "/api/runs/{id}": { get: op("Run detail with segments, chunks, event summary, evaluations and driving score", "reader", { parameters: [idParam("id")] }) },
      "/api/runs/{id}/telemetry": { post: op("Upload a telemetry chunk", "writer", { parameters: [idParam("id"), q("seq", { type: "integer" })], requestBody: jsonBody({ type: "object", properties: { samples: { type: "array", items: { $ref: "#/components/schemas/Sample" } } } }) }), get: op("Downsampled samples for playback", "reader", { parameters: [idParam("id"), q("max", { type: "integer" })] }) },
      "/api/runs/{id}/events": { post: op("Bulk event upload", "writer", { parameters: [idParam("id")], requestBody: jsonBody({ type: "object", properties: { events: { type: "array", items: { $ref: "#/components/schemas/Event" } } } }) }), get: op("List events", "reader", { parameters: [idParam("id")] }) },
      "/api/runs/{id}/finish": { post: op("Finish a run: quality report, segments, streams, privacy status; computes the driving score", "writer", { parameters: [idParam("id")], requestBody: jsonBody({ type: "object", properties: { quality: { $ref: "#/components/schemas/Quality" }, ended_at: { type: "number" } } }) }) },
      "/api/runs/{id}/evidence": { get: op("Signed evidence pack", "reader", { parameters: [idParam("id")] }) },
      "/api/runs/{id}/export/{format}": { get: op("Export a run as geojson (kepler.gl, QGIS), csv, mcap (Foxglove Studio) or json", "reader", { parameters: [idParam("id"), { name: "format", in: "path", required: true, schema: { type: "string", enum: ["geojson", "csv", "mcap", "json"] } }, q("max", { type: "integer" })] }) },
      "/api/events/critical": { get: op("Critical safety-rule violations", "reader", { parameters: [q("review_status")] }) },
      "/api/events/{id}/review": { post: op("Review an event", "writer", { parameters: [idParam("id")], requestBody: jsonBody({ type: "object", properties: { review_status: { type: "string" }, notes: { type: "string" } } }) }) },
      "/api/scenarios": { get: op("Scenario library with per-family review counts", "reader", { parameters: [q("family"), q("group"), q("review_status")] }) },
      "/api/scenarios/import": { post: op("Import a library manifest", "writer", { requestBody: jsonBody({ type: "object", properties: { scenarios: { type: "array", items: { $ref: "#/components/schemas/Scenario" } }, taxonomy_version: { type: "string" } } }) }) },
      "/api/scenarios/{id}": { get: op("Scenario detail and its runs", "reader", { parameters: [idParam("id")] }) },
      "/api/scenarios/{id}/review": { post: op("Expert alignment review", "writer", { parameters: [idParam("id")] }) },
      "/api/scenarios/{id}/export/{format}": { get: op("Export a scenario as ASAM OpenSCENARIO 1.2 (.xosc) for CARLA ScenarioRunner or esmini", "reader", { parameters: [idParam("id"), { name: "format", in: "path", required: true, schema: { type: "string", enum: ["xosc"] } }] }) },
      "/api/evaluations": { get: op("List evaluations", "reader", { parameters: [q("model_id"), q("run_id")] }), post: op("Upsert an evaluation", "writer", { requestBody: jsonBody({ $ref: "#/components/schemas/Evaluation" }) }) },
      "/api/evaluations/{id}": { get: op("Evaluation detail", "reader", { parameters: [idParam("id")] }) },
      "/api/evaluations/{id}/replay-verified": { post: op("Mark reproducibility", "writer", { parameters: [idParam("id")] }) },
      "/api/clips": { get: op("List clips", "reader", { parameters: [q("run_id")] }), post: op("Register a clip", "writer") },
      "/api/clips/{id}/object": { put: op("Upload clip bytes", "writer", { parameters: [idParam("id")] }), get: op("Download a released clip", "reader", { parameters: [idParam("id")] }) },
      "/api/clips/{id}/governance": { post: op("Redaction, consent and release state", "writer", { parameters: [idParam("id")] }) },
      "/api/fleet/live": { get: op("Latest position of every fleet device", "reader") },
      "/api/fleet/positions": { post: op("Generic position upload from edge loggers", "writer", { requestBody: jsonBody({ type: "object", properties: { positions: { type: "array", items: { $ref: "#/components/schemas/Position" } } } }) }) },
      "/api/fleet/devices/{id}/track": { get: op("Positions of one device in a time window", "reader", { parameters: [idParam("id"), q("since", { type: "number" }), q("until", { type: "number" }), q("limit", { type: "integer" })] }) },
      "/api/fleet/devices/{id}/materialize": { post: op("Create a run from a device track so it enters the catalog and KPIs", "writer", { parameters: [idParam("id")], requestBody: jsonBody({ type: "object", properties: { since: { type: "number" }, until: { type: "number" }, run_id: { type: "string" }, route_id: { type: "string" }, consent_ref: { type: "string" }, vehicle_class: { type: "string" }, notes: { type: "string" } } }) }) },
      "/api/ingest/traccar": { post: op("Traccar position/event forwarding (JSON)", "writer", { requestBody: jsonBody({ type: "object", properties: { position: { type: "object" }, device: { type: "object" }, event: { type: "object" } } }) }) },
      "/api/ingest/osmand": { get: op("OsmAnd protocol (Traccar Client, GPSLogger): id, lat, lon, timestamp, speed, bearing, altitude", "writer", { parameters: [q("id"), q("lat", { type: "number" }), q("lon", { type: "number" }), q("timestamp"), q("speed", { type: "number" }), q("bearing", { type: "number" }), q("altitude", { type: "number" }), q("token")] }), post: op("OsmAnd protocol (form body)", "writer") },
      "/api/audit": { get: op("Audit trail", "reader", { parameters: [q("limit", { type: "integer" })] }) },
    },
  };
}
