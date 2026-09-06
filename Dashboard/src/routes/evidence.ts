import type { RouteContext } from "../router";
import type { ChunkRow, RunRow } from "../types";
import { authenticate } from "../auth";
import { audit } from "../audit";
import { HttpError, canonicalJson, hmacSign, json, parseJsonColumn, requireId, sha256 } from "../util";

/**
 * Evidence pack: a signed, self-describing manifest of everything needed to
 * replay and audit one run. Partners verify the signature with the shared
 * key id; every referenced object carries its SHA-256 so the pack can be
 * checked against R2 contents later.
 */
export async function runEvidence(c: RouteContext): Promise<Response> {
  const who = await authenticate(c.request, c.env, "reader");
  const runId = requireId(c.params.id, "run_id");
  const run = await c.env.DB.prepare("SELECT * FROM runs WHERE run_id = ?1 AND tenant_id = ?2").bind(runId, who.tenant).first<RunRow>();
  if (!run) throw new HttpError(404, "run not found");
  const [chunks, events, segments, evaluations, clips, scenario] = await Promise.all([
    c.env.DB.prepare("SELECT seq, r2_key, sample_count, t_start, t_end, distance_km, sha256 FROM telemetry_chunks WHERE run_id = ?1 ORDER BY seq").bind(runId).all<ChunkRow>(),
    c.env.DB.prepare("SELECT event_id, t, frame, event_class, severity, lat, lon, description, rule_id, data, review_status, reviewed_by, reviewed_at FROM events WHERE run_id = ?1 ORDER BY t").bind(runId).all(),
    c.env.DB.prepare("SELECT idx, t_start, t_end, sample_count, passed, gates FROM segments WHERE run_id = ?1 ORDER BY idx").bind(runId).all(),
    c.env.DB.prepare("SELECT evaluation_id, model_id, model_version, scenario_id, conditions, overall, by_class, by_condition, failure_clusters, frames_evaluated, inputs, reproducible, replay_verified, created_at FROM evaluations WHERE run_id = ?1 ORDER BY created_at").bind(runId).all(),
    c.env.DB.prepare("SELECT clip_id, t_start, t_end, sha256, size_bytes, redaction_status, consent_ref, released FROM clips WHERE run_id = ?1").bind(runId).all(),
    run.scenario_id ? c.env.DB.prepare("SELECT scenario_id, family, name, content_hash, version, review_status, params FROM scenarios WHERE scenario_id = ?1 AND tenant_id = ?2").bind(run.scenario_id, who.tenant).first() : Promise.resolve(null),
  ]);
  const criticalUnreviewed = events.results.filter((e) => e.severity === "critical" && e.review_status !== "reviewed").length;
  const manifest = {
    schema: "bhutan-evidence-pack/1.0",
    generated_at: new Date().toISOString(),
    tenant: who.tenant,
    run: { ...run, bbox: run.bbox_min_lat != null ? [run.bbox_min_lat, run.bbox_min_lon, run.bbox_max_lat, run.bbox_max_lon] : null },
    scenario: scenario ? { ...scenario, params: parseJsonColumn(scenario.params, {}) } : null,
    scenario_hash_matches: scenario ? scenario.content_hash === run.scenario_hash : null,
    telemetry_chunks: chunks.results,
    segments: segments.results.map((s) => ({ ...s, gates: parseJsonColumn(s.gates, []) })),
    events: events.results.map((e) => ({ ...e, data: parseJsonColumn(e.data, null) })),
    evaluations: evaluations.results.map((e) => ({
      ...e,
      conditions: parseJsonColumn(e.conditions, {}),
      overall: parseJsonColumn(e.overall, {}),
      by_class: parseJsonColumn(e.by_class, {}),
      by_condition: parseJsonColumn(e.by_condition, {}),
      failure_clusters: parseJsonColumn(e.failure_clusters, []),
      inputs: parseJsonColumn(e.inputs, {}),
    })),
    clips: clips.results,
    attestations: {
      replay_complete: Boolean(run.replay_complete),
      quality_status: run.quality_status,
      privacy_status: run.privacy_status,
      consent_traceable: Boolean(run.consent_ref) || run.source === "sim",
      critical_events_total: events.results.filter((e) => e.severity === "critical").length,
      critical_events_unreviewed: criticalUnreviewed,
    },
  };
  const canonical = canonicalJson(manifest);
  const digest = await sha256(canonical);
  const key = c.env.MANIFEST_SIGNING_KEY;
  const signature = key ? await hmacSign(key, canonical) : null;
  await audit(c.env, who, "evidence.export", runId, { digest });
  return json({ manifest, digest, signature, key_id: signature ? c.env.MANIFEST_KEY_ID : null, signed: Boolean(signature) }, 200, {
    "content-disposition": `attachment; filename="evidence-${runId}.json"`,
  });
}
