import type { KpiReport } from "./kpi";

/**
 * Prometheus text exposition of the KPI report so Grafana (or any
 * Prometheus-compatible scraper) can graph and alert on the same numbers the
 * dashboard shows. Scrape with `bearer_token` set to a reader token.
 */
export function renderPrometheus(report: KpiReport, extra: Record<string, number> = {}): string {
  const lines: string[] = [];
  const label = (pairs: Record<string, string>) => "{" + Object.entries(pairs).map(([k, v]) => `${k}="${escapeLabel(v)}"`).join(",") + "}";
  lines.push("# HELP atlas_kpi_value Current value of a Bhutan Atlas KPI.");
  lines.push("# TYPE atlas_kpi_value gauge");
  for (const k of report.kpis) {
    if (k.value === null) continue;
    lines.push(`atlas_kpi_value${label({ tenant: report.tenant, kpi: k.id, unit: k.unit })} ${k.value}`);
  }
  lines.push("# HELP atlas_kpi_on_track 1 when the KPI meets its target, 0 when at risk; absent while there is no data.");
  lines.push("# TYPE atlas_kpi_on_track gauge");
  for (const k of report.kpis) {
    if (k.status === "no_data") continue;
    lines.push(`atlas_kpi_on_track${label({ tenant: report.tenant, kpi: k.id })} ${k.status === "on_track" ? 1 : 0}`);
  }
  lines.push("# HELP atlas_edge_case_events_total Events logged per class.");
  lines.push("# TYPE atlas_edge_case_events_total counter");
  lines.push("# HELP atlas_edge_case_rate_per_100km Events per 100 km per class.");
  lines.push("# TYPE atlas_edge_case_rate_per_100km gauge");
  for (const e of report.edge_case_rates) {
    lines.push(`atlas_edge_case_events_total${label({ tenant: report.tenant, event_class: e.event_class })} ${e.count}`);
    lines.push(`atlas_edge_case_rate_per_100km${label({ tenant: report.tenant, event_class: e.event_class })} ${e.per_100km}`);
  }
  lines.push("# HELP atlas_perception_metric Precision, recall and F1 of the latest models.");
  lines.push("# TYPE atlas_perception_metric gauge");
  for (const m of report.perception.models) {
    for (const metric of ["precision", "recall", "f1"] as const) {
      const v = m.overall[metric];
      if (typeof v === "number") lines.push(`atlas_perception_metric${label({ tenant: report.tenant, model: m.model_id, version: m.model_version || "", metric })} ${v}`);
    }
  }
  for (const [name, value] of Object.entries(extra)) {
    lines.push(`# TYPE ${name} gauge`);
    lines.push(`${name}${label({ tenant: report.tenant })} ${value}`);
  }
  lines.push(`# HELP atlas_kpi_computed_timestamp_seconds Unix time the report was computed.`);
  lines.push(`# TYPE atlas_kpi_computed_timestamp_seconds gauge`);
  lines.push(`atlas_kpi_computed_timestamp_seconds${label({ tenant: report.tenant })} ${Math.floor(Date.parse(report.computed_at) / 1000)}`);
  return lines.join("\n") + "\n";
}

function escapeLabel(value: string): string {
  return value.replace(/\\/g, "\\\\").replace(/"/g, '\\"').replace(/\n/g, "\\n");
}
