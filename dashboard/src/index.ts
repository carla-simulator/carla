import type { Env } from "./types";
import { Router } from "./router";
import { HttpError, json } from "./util";
import { snapshotKpis } from "./kpi";
import { parseTokens } from "./auth";
import { createRun, finishRun, getRun, getTelemetry, listCriticalEvents, listEvents, listRuns, reviewEvent, uploadEvents, uploadTelemetry } from "./routes/runs";
import { getScenario, importScenarios, listScenarios, reviewScenario } from "./routes/scenarios";
import { createEvaluation, getEvaluation, listEvaluations, markReplayVerified } from "./routes/evaluations";
import { getClipObject, listClips, putClipObject, registerClip, updateClipGovernance } from "./routes/clips";
import { getKpiHistory, getKpis, getMetrics, getOpenApi, listAudit, postKpiSnapshot } from "./routes/kpis";
import { runEvidence } from "./routes/evidence";
import { exportRun, exportScenarioXosc } from "./routes/exports";
import { deviceTrack, ingestOsmAnd, ingestTraccar, listFleet, materializeTrack, postPositions } from "./routes/fleet";

const router = new Router()
  .get("/api/health", (c) => json({ ok: true, app: c.env.APP_NAME, time: new Date().toISOString() }))
  .get("/api/openapi.json", getOpenApi)
  .get("/api/metrics", getMetrics)
  .get("/api/kpis", getKpis)
  .get("/api/kpis/history", getKpiHistory)
  .post("/api/kpis/snapshot", postKpiSnapshot)
  .get("/api/runs", listRuns)
  .post("/api/runs", createRun)
  .get("/api/runs/:id", getRun)
  .post("/api/runs/:id/telemetry", uploadTelemetry)
  .get("/api/runs/:id/telemetry", getTelemetry)
  .post("/api/runs/:id/events", uploadEvents)
  .get("/api/runs/:id/events", listEvents)
  .post("/api/runs/:id/finish", finishRun)
  .get("/api/runs/:id/evidence", runEvidence)
  .get("/api/runs/:id/export/:format", exportRun)
  .get("/api/events/critical", listCriticalEvents)
  .post("/api/events/:id/review", reviewEvent)
  .get("/api/scenarios", listScenarios)
  .post("/api/scenarios/import", importScenarios)
  .get("/api/scenarios/:id", getScenario)
  .post("/api/scenarios/:id/review", reviewScenario)
  .get("/api/scenarios/:id/export/:format", exportScenarioXosc)
  .get("/api/fleet/live", listFleet)
  .post("/api/fleet/positions", postPositions)
  .get("/api/fleet/devices/:id/track", deviceTrack)
  .post("/api/fleet/devices/:id/materialize", materializeTrack)
  .post("/api/ingest/traccar", ingestTraccar)
  .get("/api/ingest/osmand", ingestOsmAnd)
  .post("/api/ingest/osmand", ingestOsmAnd)
  .get("/api/evaluations", listEvaluations)
  .post("/api/evaluations", createEvaluation)
  .get("/api/evaluations/:id", getEvaluation)
  .post("/api/evaluations/:id/replay-verified", markReplayVerified)
  .get("/api/clips", listClips)
  .post("/api/clips", registerClip)
  .put("/api/clips/:id/object", putClipObject)
  .get("/api/clips/:id/object", getClipObject)
  .post("/api/clips/:id/governance", updateClipGovernance)
  .get("/api/audit", listAudit);

const CORS_HEADERS = {
  "access-control-allow-origin": "*",
  "access-control-allow-methods": "GET, POST, PUT, OPTIONS",
  "access-control-allow-headers": "authorization, content-type, x-tenant",
  "access-control-max-age": "86400",
};

export default {
  async fetch(request: Request, env: Env, ctx: ExecutionContext): Promise<Response> {
    const url = new URL(request.url);
    if (!url.pathname.startsWith("/api/")) {
      return env.ASSETS.fetch(request);
    }
    if (request.method === "OPTIONS") {
      return new Response(null, { status: 204, headers: CORS_HEADERS });
    }
    const match = router.match(request.method, url.pathname);
    if (!match) {
      return json({ error: "not found" }, 404, CORS_HEADERS);
    }
    try {
      const response = await match.handler({ request, env, ctx, params: match.params, url });
      const headers = new Headers(response.headers);
      for (const [k, v] of Object.entries(CORS_HEADERS)) headers.set(k, v);
      headers.set("x-content-type-options", "nosniff");
      return new Response(response.body, { status: response.status, headers });
    } catch (error) {
      if (error instanceof HttpError) {
        return json({ error: error.message, detail: error.detail ?? null }, error.status, CORS_HEADERS);
      }
      console.error("unhandled error", error);
      return json({ error: "internal error" }, 500, CORS_HEADERS);
    }
  },

  /** Nightly KPI snapshots for every tenant that has a token configured. */
  async scheduled(_controller: ScheduledController, env: Env, ctx: ExecutionContext): Promise<void> {
    const tenants = new Set<string>([env.DEFAULT_TENANT]);
    for (const entry of parseTokens(env.API_TOKENS).values()) tenants.add(entry.tenant);
    for (const tenant of tenants) {
      ctx.waitUntil(snapshotKpis(env, tenant).catch((err) => console.error("kpi snapshot failed", tenant, err)));
    }
  },
} satisfies ExportedHandler<Env>;
