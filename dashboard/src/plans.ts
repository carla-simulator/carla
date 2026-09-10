/**
 * Saved budget plans.
 *
 * A saved plan stores only its planner *input*. Every read re-runs
 * `planProgram` over it, so a plan saved last quarter is priced at today's
 * reference rates rather than quietly serving stale numbers; the totals taken
 * at save time are kept beside it so the movement is visible instead of silent.
 * Two plans can then be diffed field by field — the review a budget change
 * needs — and either one exported as CSV or as a Markdown section of the
 * partner report.
 */

import { GPUS, PROGRAMS, RESOLUTIONS, type Plan, type PlannerInput, planProgram } from "./planner";

/** The headline numbers of a plan: what a budget line actually quotes. */
export interface PlanTotals {
  scenes: number;
  frames: number;
  total_gb: number;
  gpu: string;
  usd_per_gpu_hour: number;
  gpu_hours: number;
  wall_clock_days: number;
  compute_usd: number;
  storage_usd: number;
  total_usd: number;
  total_usd_low: number;
  total_usd_high: number;
}

export interface FieldDiff {
  field: string;
  label: string;
  a: string | number | boolean | null;
  b: string | number | boolean | null;
}

export interface MetricDiff {
  metric: keyof PlanTotals;
  label: string;
  unit: string;
  a: number;
  b: number;
  delta: number;
  /** Percentage change from a to b; null when a is 0 and the ratio is undefined. */
  pct: number | null;
}

export interface SavedPlan {
  plan_id: string;
  name: string;
  notes: string | null;
  input: PlannerInput;
  /** Totals as computed when the plan was saved. */
  saved: PlanTotals;
  /** Totals from today's reference tables. */
  current: PlanTotals;
  /** Where the two disagree, i.e. what moved under the plan since it was saved. */
  drift: MetricDiff[];
  created_by: string | null;
  created_at: string;
  updated_at: string | null;
}

export interface PlanComparison {
  a: { plan_id: string; name: string };
  b: { plan_id: string; name: string };
  input: FieldDiff[];
  metrics: MetricDiff[];
}

const INPUT_LABELS: Record<keyof PlannerInput, string> = {
  scenes: "Scenes",
  clip_seconds: "Seconds per scene",
  fps: "Frames per second",
  cameras: "Cameras",
  resolution: "Resolution",
  program: "Training program",
  gpu: "GPU",
  gpus: "GPUs in parallel",
  interruptible: "Interruptible instance",
  usd_per_gpu_hour: "$/GPU-hour override",
  usd_per_gb_month: "$/GB-month",
  retention_months: "Retention (months)",
};

const METRIC_LABELS: Array<{ metric: keyof PlanTotals; label: string; unit: string }> = [
  { metric: "scenes", label: "Scenes", unit: "scenes" },
  { metric: "frames", label: "Frames", unit: "frames" },
  { metric: "total_gb", label: "Storage", unit: "GB" },
  { metric: "gpu_hours", label: "GPU-hours (expected)", unit: "h" },
  { metric: "wall_clock_days", label: "Wall clock (expected)", unit: "days" },
  { metric: "usd_per_gpu_hour", label: "Rate", unit: "USD/h" },
  { metric: "compute_usd", label: "Compute cost (expected)", unit: "USD" },
  { metric: "storage_usd", label: "Storage cost", unit: "USD" },
  { metric: "total_usd", label: "Total (expected)", unit: "USD" },
];

/** A plan id a URL and `requireId` both accept, derived from the name so links stay readable. */
export function slugPlanId(name: string): string {
  const slug = name
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "")
    .slice(0, 60)
    .replace(/-+$/, "");
  return slug || "plan-" + Math.random().toString(36).slice(2, 10);
}

export function summarize(plan: Plan): PlanTotals {
  return {
    scenes: plan.dataset.scenes,
    frames: plan.dataset.frames,
    total_gb: plan.dataset.total_gb,
    gpu: plan.compute.gpu,
    usd_per_gpu_hour: plan.compute.usd_per_gpu_hour,
    gpu_hours: plan.compute.gpu_hours.expected,
    wall_clock_days: plan.compute.wall_clock_days.expected,
    compute_usd: plan.compute.cost_usd.expected,
    storage_usd: plan.storage.cost_usd,
    total_usd: plan.total_usd.expected,
    total_usd_low: plan.total_usd.low,
    total_usd_high: plan.total_usd.high,
  };
}

/** Numeric metrics that differ between two sets of totals, newest-first callers included. */
export function diffTotals(a: PlanTotals, b: PlanTotals): MetricDiff[] {
  const out: MetricDiff[] = [];
  for (const { metric, label, unit } of METRIC_LABELS) {
    const av = a[metric];
    const bv = b[metric];
    if (typeof av !== "number" || typeof bv !== "number" || av === bv) continue;
    const delta = Math.round((bv - av) * 10000) / 10000;
    out.push({ metric, label, unit, a: av, b: bv, delta, pct: av === 0 ? null : Math.round(((bv - av) / av) * 1000) / 10 });
  }
  return out;
}

function displayInput(input: PlannerInput, field: keyof PlannerInput): string | number | boolean | null {
  const value = input[field];
  if (field === "gpu") return GPUS[input.gpu].label;
  if (field === "program") return PROGRAMS[input.program].label;
  if (field === "resolution") return RESOLUTIONS[input.resolution].label;
  return value as string | number | boolean | null;
}

/** Which inputs changed between two plans, and what that did to the numbers. */
export function comparePlanRecords(a: SavedPlan, b: SavedPlan): PlanComparison {
  const input: FieldDiff[] = [];
  for (const field of Object.keys(INPUT_LABELS) as Array<keyof PlannerInput>) {
    if (a.input[field] === b.input[field]) continue;
    input.push({ field, label: INPUT_LABELS[field], a: displayInput(a.input, field), b: displayInput(b.input, field) });
  }
  return {
    a: { plan_id: a.plan_id, name: a.name },
    b: { plan_id: b.plan_id, name: b.name },
    input,
    metrics: diffTotals(a.current, b.current),
  };
}

function csvCell(value: unknown): string {
  const text = value === null || value === undefined ? "" : String(value);
  return /[",\n]/.test(text) ? '"' + text.replace(/"/g, '""') + '"' : text;
}

/**
 * Budget as CSV: one line item per row, so it drops straight into a
 * spreadsheet next to the rest of a pilot budget.
 */
export function planToCsv(record: SavedPlan, plan: Plan): string {
  const rows: Array<[string, string, string | number, string]> = [
    ["plan", "name", record.name, ""],
    ["plan", "plan_id", record.plan_id, ""],
    ["plan", "saved_at", record.created_at, ""],
    ["plan", "tier", plan.tier.label, plan.tier.range],
    ["input", "scenes", plan.input.scenes, "scenes"],
    ["input", "seconds per scene", plan.input.clip_seconds, "s"],
    ["input", "frames per second", plan.input.fps, "fps"],
    ["input", "cameras", plan.input.cameras, "cameras"],
    ["input", "resolution", RESOLUTIONS[plan.input.resolution].label, `${RESOLUTIONS[plan.input.resolution].mbps} Mbit/s`],
    ["input", "training program", PROGRAMS[plan.input.program].label, ""],
    ["input", "gpu", plan.compute.gpu, plan.compute.interruptible ? "interruptible" : "on-demand"],
    ["input", "gpus in parallel", plan.compute.gpus, "GPUs"],
    ["dataset", "frames", plan.dataset.frames, "frames"],
    ["dataset", "video", plan.dataset.video_hours, "h"],
    ["dataset", "raw video", plan.dataset.raw_gb, "GB"],
    ["dataset", "labels and manifests", plan.dataset.labels_gb, "GB"],
    ["dataset", "working copy", plan.dataset.working_gb, "GB"],
    ["dataset", "total storage", plan.dataset.total_gb, "GB"],
    ["compute", "rate", plan.compute.usd_per_gpu_hour, "USD/h"],
    ["compute", "gpu-hours (low)", plan.compute.gpu_hours.low, "h"],
    ["compute", "gpu-hours (expected)", plan.compute.gpu_hours.expected, "h"],
    ["compute", "gpu-hours (high)", plan.compute.gpu_hours.high, "h"],
    ["compute", "wall clock (expected)", plan.compute.wall_clock_days.expected, "days"],
    ["cost", "compute (low)", plan.compute.cost_usd.low, "USD"],
    ["cost", "compute (expected)", plan.compute.cost_usd.expected, "USD"],
    ["cost", "compute (high)", plan.compute.cost_usd.high, "USD"],
    ["cost", "storage", plan.storage.cost_usd, "USD"],
    ["cost", "total (low)", plan.total_usd.low, "USD"],
    ["cost", "total (expected)", plan.total_usd.expected, "USD"],
    ["cost", "total (high)", plan.total_usd.high, "USD"],
  ];
  return ["section,item,value,unit", ...rows.map((r) => r.map(csvCell).join(","))].join("\n") + "\n";
}

/** Budget as a Markdown section, ready to paste into the partner report. */
export function planToMarkdown(record: SavedPlan, plan: Plan): string {
  const usd = (n: number) => "$" + Math.round(n).toLocaleString("en-US");
  const int = (n: number) => Math.round(n).toLocaleString("en-US");
  const lines = [
    `# Budget — ${record.name}`,
    "",
    record.notes ? record.notes + "\n" : "",
    `**${plan.tier.label}** (${plan.tier.range}). ${plan.tier.note}`,
    "",
    "## Corpus",
    "",
    "| Item | Value |",
    "|---|---|",
    `| Scenes | ${int(plan.dataset.scenes)} × ${plan.input.clip_seconds} s at ${plan.input.fps} fps |`,
    `| Cameras | ${plan.input.cameras} × ${RESOLUTIONS[plan.input.resolution].label} (${RESOLUTIONS[plan.input.resolution].mbps} Mbit/s) |`,
    `| Frames | ${int(plan.dataset.frames)} |`,
    `| Video | ${plan.dataset.video_hours} h |`,
    `| Storage | ${plan.dataset.total_gb} GB (${plan.dataset.raw_gb} GB raw + ${plan.dataset.labels_gb} GB labels + ${plan.dataset.working_gb} GB working) |`,
    "",
    "## Compute and cost",
    "",
    "| Line | Low | Expected | High |",
    "|---|---:|---:|---:|",
    `| GPU-hours on ${plan.compute.gpu} | ${int(plan.compute.gpu_hours.low)} | ${int(plan.compute.gpu_hours.expected)} | ${int(plan.compute.gpu_hours.high)} |`,
    `| Compute at $${plan.compute.usd_per_gpu_hour}/h${plan.compute.interruptible ? " (interruptible)" : ""} | ${usd(plan.compute.cost_usd.low)} | ${usd(plan.compute.cost_usd.expected)} | ${usd(plan.compute.cost_usd.high)} |`,
    `| Storage, ${plan.storage.months} month(s) at $${plan.storage.usd_per_gb_month}/GB | ${usd(plan.storage.cost_usd)} | ${usd(plan.storage.cost_usd)} | ${usd(plan.storage.cost_usd)} |`,
    `| **Total** | **${usd(plan.total_usd.low)}** | **${usd(plan.total_usd.expected)}** | **${usd(plan.total_usd.high)}** |`,
    "",
    `Wall clock: ${plan.compute.wall_clock_days.expected} days on ${plan.compute.gpus} × ${plan.compute.gpu} at ${Math.round(plan.compute.parallel_efficiency * 100)} % scaling efficiency.`,
    "",
    "## Assumptions",
    "",
    ...plan.assumptions.map((a) => `* ${a}`),
    "",
    `_Plan \`${record.plan_id}\` saved ${record.created_at}; figures recomputed ${new Date().toISOString()}._`,
    "",
  ];
  return lines.filter((line, i) => line !== "" || lines[i - 1] !== "").join("\n");
}

/** Recompute a stored row into the shape the API serves. */
export function hydrate(row: { plan_id: string; name: string; notes: string | null; input: PlannerInput; saved: PlanTotals; created_by: string | null; created_at: string; updated_at: string | null }): SavedPlan {
  const current = summarize(planProgram(row.input));
  return {
    plan_id: row.plan_id,
    name: row.name,
    notes: row.notes,
    input: row.input,
    saved: row.saved,
    current,
    drift: diffTotals(row.saved, current),
    created_by: row.created_by,
    created_at: row.created_at,
    updated_at: row.updated_at,
  };
}
