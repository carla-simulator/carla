import type { RouteContext } from "../router";
import type { RunRow } from "../types";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { HttpError, json, parseJsonColumn, requireId } from "../util";
import { runToGeoJson, runToMcap, samplesToCsv, type ExportEvent } from "../exports";
import { scenarioToXosc, type XoscScenario } from "../openscenario";
import { loadRunSamples } from "./runs";

const FORMATS = new Set(["geojson", "csv", "mcap", "json"]);

/** GET /api/runs/:id/export/:format  (geojson | csv | mcap | json) */
export async function exportRun(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const runId = requireId(c.params.id, "run_id");
  const format = c.params.format;
  if (!FORMATS.has(format)) throw new HttpError(400, `unsupported format '${format}'; use ${[...FORMATS].join(", ")}`);
  const run = await c.env.DB.prepare("SELECT * FROM runs WHERE run_id = ?1 AND tenant_id = ?2").bind(runId, who.tenant).first<RunRow>();
  if (!run) throw new HttpError(404, "run not found");
  const maxPoints = Math.min(200000, Math.max(100, Number(c.url.searchParams.get("max") || 100000)));
  const [samples, events] = await Promise.all([
    loadRunSamples(c.env, who.tenant, runId, maxPoints),
    c.env.DB.prepare("SELECT event_id, t, frame, event_class, severity, lat, lon, description, rule_id, data, review_status FROM events WHERE run_id = ?1 AND tenant_id = ?2 ORDER BY t").bind(runId, who.tenant).all<ExportEvent & { data: string | null }>(),
  ]);
  const eventList: ExportEvent[] = events.results.map((e) => ({ ...e, data: parseJsonColumn(e.data, null) }));
  await audit(c.env, who, "run.export", runId, { format, samples: samples.length });
  const disposition = (ext: string) => ({ "content-disposition": `attachment; filename="${runId}.${ext}"` });
  switch (format) {
    case "geojson":
      return new Response(JSON.stringify(runToGeoJson(run, samples, eventList)), { headers: { "content-type": "application/geo+json; charset=utf-8", ...disposition("geojson") } });
    case "csv":
      return new Response(samplesToCsv(samples), { headers: { "content-type": "text/csv; charset=utf-8", ...disposition("csv") } });
    case "mcap": {
      const bytes = await runToMcap(run, samples, eventList);
      return new Response(bytes, { headers: { "content-type": "application/octet-stream", "content-length": String(bytes.byteLength), ...disposition("mcap") } });
    }
    default:
      return json({ run, samples, events: eventList }, 200, disposition("json"));
  }
}

/** GET /api/scenarios/:id/export/xosc */
export async function exportScenarioXosc(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const id = requireId(c.params.id, "scenario_id");
  if (c.params.format !== "xosc") throw new HttpError(400, "unsupported format; use xosc");
  const row = await c.env.DB.prepare("SELECT * FROM scenarios WHERE scenario_id = ?1 AND tenant_id = ?2").bind(id, who.tenant).first<Record<string, unknown>>();
  if (!row) throw new HttpError(404, "scenario not found");
  const scenario: XoscScenario = {
    scenario_id: String(row.scenario_id),
    family: String(row.family),
    name: String(row.name || row.scenario_id),
    description: (row.description as string | null) ?? null,
    version: (row.version as string | null) ?? null,
    content_hash: (row.content_hash as string | null) ?? null,
    params: parseJsonColumn<Record<string, unknown>>(row.params, {}),
    actors: parseJsonColumn<Array<Record<string, unknown>>>(row.actors, []),
    expected_events: parseJsonColumn<string[]>(row.expected_events, []),
  };
  const xml = scenarioToXosc(scenario);
  return new Response(xml, { headers: { "content-type": "application/xml; charset=utf-8", "content-disposition": `attachment; filename="${id}.xosc"` } });
}
