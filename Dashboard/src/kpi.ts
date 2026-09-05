import type { Counts, Env } from "./types";
import { parseJsonColumn } from "./util";

export type KpiStatus = "on_track" | "at_risk" | "no_data";

export interface Kpi {
  id: string;
  label: string;
  value: number | null;
  unit: string;
  target: string;
  status: KpiStatus;
  why: string;
  detail?: unknown;
}

export interface KpiReport {
  tenant: string;
  computed_at: string;
  kpis: Kpi[];
  edge_case_rates: Array<{ event_class: string; count: number; per_100km: number }>;
  perception: { models: Array<{ model_id: string; model_version: string | null; evaluations: number; overall: Counts; by_class: Record<string, Counts>; by_condition: Record<string, Record<string, Counts>> }> };
}

function pct(n: number, d: number): number | null {
  return d > 0 ? Math.round((n / d) * 1000) / 10 : null;
}

function statusMin(value: number | null, min: number): KpiStatus {
  if (value === null) return "no_data";
  return value >= min ? "on_track" : "at_risk";
}

function withRates(c: Counts): Counts {
  const precision = c.tp + c.fp > 0 ? c.tp / (c.tp + c.fp) : 0;
  const recall = c.tp + c.fn > 0 ? c.tp / (c.tp + c.fn) : 0;
  const f1 = precision + recall > 0 ? (2 * precision * recall) / (precision + recall) : 0;
  return { tp: c.tp, fp: c.fp, fn: c.fn, precision: +precision.toFixed(4), recall: +recall.toFixed(4), f1: +f1.toFixed(4) };
}

function add(target: Counts, source: Counts | undefined): void {
  if (!source) return;
  target.tp += source.tp || 0;
  target.fp += source.fp || 0;
  target.fn += source.fn || 0;
}

export async function computeKpis(env: Env, tenant: string): Promise<KpiReport> {
  const db = env.DB;
  const [coverage, replay, scenarios, families, segments, distance, events, evals, critical, clips, evalRows] = await Promise.all([
    db.prepare("SELECT COALESCE(SUM(CASE WHEN source = 'vehicle' AND quality_status = 'accepted' THEN duration_s ELSE 0 END), 0) AS vehicle_s, COALESCE(SUM(CASE WHEN source = 'sim' THEN duration_s ELSE 0 END), 0) AS sim_s, COUNT(*) AS runs FROM runs WHERE tenant_id = ?1").bind(tenant).first<{ vehicle_s: number; sim_s: number; runs: number }>(),
    db.prepare("SELECT COUNT(*) AS finished, SUM(replay_complete) AS complete FROM runs WHERE tenant_id = ?1 AND ended_at IS NOT NULL").bind(tenant).first<{ finished: number; complete: number | null }>(),
    db.prepare("SELECT COUNT(*) AS n FROM scenarios WHERE tenant_id = ?1").bind(tenant).first<{ n: number }>(),
    db.prepare("SELECT COUNT(DISTINCT family) AS total, COUNT(DISTINCT CASE WHEN review_status = 'reviewed' THEN family END) AS reviewed FROM scenarios WHERE tenant_id = ?1").bind(tenant).first<{ total: number; reviewed: number }>(),
    db.prepare("SELECT COUNT(*) AS n, SUM(passed) AS passed FROM segments WHERE tenant_id = ?1").bind(tenant).first<{ n: number; passed: number | null }>(),
    db.prepare("SELECT COALESCE(SUM(distance_km), 0) AS km FROM runs WHERE tenant_id = ?1").bind(tenant).first<{ km: number }>(),
    db.prepare("SELECT event_class, COUNT(*) AS n FROM events WHERE tenant_id = ?1 GROUP BY event_class ORDER BY n DESC").bind(tenant).all<{ event_class: string; n: number }>(),
    db.prepare("SELECT COUNT(*) AS n, SUM(reproducible) AS reproducible, SUM(replay_verified) AS verified FROM evaluations WHERE tenant_id = ?1").bind(tenant).first<{ n: number; reproducible: number | null; verified: number | null }>(),
    db.prepare("SELECT COUNT(*) AS n, SUM(CASE WHEN rule_id IS NOT NULL OR review_status != 'unreviewed' THEN 1 ELSE 0 END) AS classified, SUM(CASE WHEN review_status = 'reviewed' THEN 1 ELSE 0 END) AS reviewed FROM events WHERE tenant_id = ?1 AND severity = 'critical'").bind(tenant).first<{ n: number; classified: number | null; reviewed: number | null }>(),
    db.prepare("SELECT COUNT(*) AS released, SUM(CASE WHEN redaction_status IN ('redacted', 'not_required') AND consent_ref IS NOT NULL THEN 1 ELSE 0 END) AS compliant FROM clips WHERE tenant_id = ?1 AND released = 1").bind(tenant).first<{ released: number; compliant: number | null }>(),
    db.prepare("SELECT model_id, model_version, overall, by_class, by_condition FROM evaluations WHERE tenant_id = ?1 ORDER BY created_at DESC LIMIT 500").bind(tenant).all<{ model_id: string; model_version: string | null; overall: string; by_class: string; by_condition: string }>(),
  ]);

  const vehicleHours = Math.round(((coverage?.vehicle_s || 0) / 3600) * 10) / 10;
  const simHours = Math.round(((coverage?.sim_s || 0) / 3600) * 10) / 10;
  const replayPct = pct(replay?.complete || 0, replay?.finished || 0);
  const scenarioCount = scenarios?.n || 0;
  const reviewedFamilies = families?.reviewed || 0;
  const acceptance = pct(segments?.passed || 0, segments?.n || 0);
  const totalKm = distance?.km || 0;
  const reproPct = pct(evals?.reproducible || 0, evals?.n || 0);
  const criticalN = critical?.n || 0;
  const criticalClassified = pct(critical?.classified || 0, criticalN);
  const criticalReviewed = pct(critical?.reviewed || 0, criticalN);
  const governance = pct(clips?.compliant || 0, clips?.released || 0);

  const kpis: Kpi[] = [
    { id: "route_coverage", label: "Real-world route coverage", value: vehicleHours, unit: "h", target: "50-100 h", status: coverage?.runs ? statusMin(vehicleHours, 50) : "no_data", why: "Establishes local data provenance", detail: { sim_hours: simHours, runs: coverage?.runs || 0 } },
    { id: "replay_completeness", label: "Replay completeness", value: replayPct, unit: "%", target: ">=95 %", status: statusMin(replayPct, 95), why: "Enables auditability", detail: { finished_runs: replay?.finished || 0 } },
    { id: "scenario_coverage", label: "Scenario-library coverage", value: scenarioCount, unit: "scenarios", target: "100+", status: scenarioCount ? statusMin(scenarioCount, 100) : "no_data", why: "Proves synthetic breadth", detail: { families: families?.total || 0 } },
    { id: "alignment_review", label: "Synthetic-to-real alignment review", value: reviewedFamilies, unit: "families", target: "20+ reviewed", status: families?.total ? statusMin(reviewedFamilies, 20) : "no_data", why: "Measures whether simulation reflects local risk patterns" },
    { id: "quality_acceptance", label: "Data-quality acceptance rate", value: acceptance, unit: "%", target: ">=90 %", status: statusMin(acceptance, 90), why: "Prevents unusable training data", detail: { segments: segments?.n || 0 } },
    { id: "edge_case_rate", label: "Edge-case discovery rate", value: totalKm > 0 ? Math.round(((events.results.reduce((n, e) => n + e.n, 0) * 100) / totalKm) * 10) / 10 : null, unit: "events/100 km", target: "tracked by class", status: totalKm > 0 ? "on_track" : "no_data", why: "Identifies high-value collection gaps", detail: { total_km: Math.round(totalKm * 100) / 100 } },
    { id: "evaluation_reproducibility", label: "Evaluation reproducibility", value: reproPct, unit: "%", target: "100 %", status: reproPct === null ? "no_data" : reproPct >= 100 ? "on_track" : "at_risk", why: "Essential for customer and regulator trust", detail: { evaluations: evals?.n || 0, replay_verified: evals?.verified || 0 } },
    { id: "critical_violations", label: "Critical safety-rule violations reviewed", value: criticalN ? criticalReviewed : null, unit: "%", target: "100 % logged, classified, reviewed", status: criticalN === 0 ? "no_data" : criticalReviewed !== null && criticalReviewed >= 100 ? "on_track" : "at_risk", why: "Supports a disciplined safety process", detail: { logged: criticalN, classified_pct: criticalClassified } },
    { id: "governance_compliance", label: "Data-governance compliance", value: governance, unit: "%", target: "100 % of released clips", status: governance === null ? "no_data" : governance >= 100 ? "on_track" : "at_risk", why: "Makes later data sales viable", detail: { released_clips: clips?.released || 0 } },
  ];

  const edgeCaseRates = events.results.map((e) => ({ event_class: e.event_class, count: e.n, per_100km: totalKm > 0 ? Math.round(((e.n * 100) / totalKm) * 100) / 100 : 0 }));

  const models = new Map<string, { model_id: string; model_version: string | null; evaluations: number; overall: Counts; by_class: Record<string, Counts>; by_condition: Record<string, Record<string, Counts>> }>();
  for (const row of evalRows.results) {
    const key = `${row.model_id}@${row.model_version || ""}`;
    let entry = models.get(key);
    if (!entry) {
      entry = { model_id: row.model_id, model_version: row.model_version, evaluations: 0, overall: { tp: 0, fp: 0, fn: 0 }, by_class: {}, by_condition: {} };
      models.set(key, entry);
    }
    entry.evaluations += 1;
    add(entry.overall, parseJsonColumn<Counts>(row.overall, { tp: 0, fp: 0, fn: 0 }));
    for (const [cls, counts] of Object.entries(parseJsonColumn<Record<string, Counts>>(row.by_class, {}))) {
      entry.by_class[cls] = entry.by_class[cls] || { tp: 0, fp: 0, fn: 0 };
      add(entry.by_class[cls], counts);
    }
    for (const [dim, values] of Object.entries(parseJsonColumn<Record<string, Record<string, Counts>>>(row.by_condition, {}))) {
      entry.by_condition[dim] = entry.by_condition[dim] || {};
      for (const [val, counts] of Object.entries(values)) {
        entry.by_condition[dim][val] = entry.by_condition[dim][val] || { tp: 0, fp: 0, fn: 0 };
        add(entry.by_condition[dim][val], counts);
      }
    }
  }
  const perception = {
    models: Array.from(models.values()).map((m) => ({
      ...m,
      overall: withRates(m.overall),
      by_class: Object.fromEntries(Object.entries(m.by_class).map(([k, v]) => [k, withRates(v)])),
      by_condition: Object.fromEntries(Object.entries(m.by_condition).map(([dim, vals]) => [dim, Object.fromEntries(Object.entries(vals).map(([k, v]) => [k, withRates(v)]))])),
    })),
  };
  kpis.splice(6, 0, {
    id: "perception_benchmark",
    label: "Perception benchmark",
    value: perception.models.length ? perception.models[0].overall.recall! * 100 : null,
    unit: "% recall (latest model)",
    target: "baseline by object, weather, lighting, route",
    status: perception.models.length ? "on_track" : "no_data",
    why: "Makes the product useful to model teams",
    detail: { models: perception.models.length },
  });

  return { tenant, computed_at: new Date().toISOString(), kpis, edge_case_rates: edgeCaseRates, perception };
}

export async function snapshotKpis(env: Env, tenant: string): Promise<KpiReport> {
  const report = await computeKpis(env, tenant);
  await env.DB.prepare("INSERT INTO kpi_snapshots (snapshot_id, tenant_id, computed_at, payload) VALUES (?1, ?2, ?3, ?4)")
    .bind(`${tenant}:${report.computed_at}`, tenant, report.computed_at, JSON.stringify({ kpis: report.kpis, edge_case_rates: report.edge_case_rates }))
    .run();
  return report;
}
