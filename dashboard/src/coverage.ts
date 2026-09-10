/**
 * ODD coverage matrix.
 *
 * Answers "which parts of the operational design domain does the catalog
 * actually cover?" by crossing the scenario library's visibility_class with
 * its lighting_class and counting, per cell, the planned scenario variants and
 * the runs recorded against them.
 *
 * What closing a gap *costs* is a separate question, answered by src/planner.ts;
 * this module deliberately holds no pricing model.
 *
 * Everything here is a pure function over rows so it can be unit-tested
 * without D1; the route layer in routes/coverage.ts supplies the rows.
 */

/** Preferred axis order; anything else the catalog contains is appended. */
const VISIBILITY_ORDER = ["clear", "wet", "rain", "heavy_rain", "fog", "snow"];
const LIGHTING_ORDER = ["daylight", "low_light", "night"];
const UNLABELLED = "unlabelled";

export interface ScenarioCoverageRow {
  route_class: string | null;
  lighting_class: string | null;
  visibility_class: string | null;
  review_status?: string | null;
  variants?: number;
}

export interface RunCoverageRow {
  route_class: string | null;
  lighting_class: string | null;
  visibility_class: string | null;
  source?: string;
  quality_status?: string;
  duration_s?: number;
  distance_km?: number;
  runs?: number;
}

export type CellStatus = "covered" | "thin" | "gap";

export interface CoverageCell {
  visibility: string;
  lighting: string;
  scenarios: number;
  reviewed: number;
  runs: number;
  accepted_runs: number;
  duration_s: number;
  distance_km: number;
  status: CellStatus;
  /**
   * False when either axis is `unlabelled` — runs with no scenario behind them.
   * Those cells are shown so the data is visible, but they are not ODD
   * combinations, so they do not count towards coverage or gaps.
   */
  odd: boolean;
}

export interface CoverageMatrix {
  axes: { visibility: string[]; lighting: string[] };
  cells: CoverageCell[];
  by_route_class: Array<{ route_class: string; scenarios: number; runs: number; accepted_runs: number; duration_s: number }>;
  gaps: Array<{ visibility: string; lighting: string; status: CellStatus; scenarios: number; runs: number }>;
  totals: {
    scenarios: number;
    runs: number;
    accepted_runs: number;
    duration_s: number;
    distance_km: number;
    cells: number;
    covered_cells: number;
    thin_cells: number;
    gap_cells: number;
    coverage_pct: number;
  };
  /** A cell needs this many accepted runs before it counts as covered. */
  min_runs_per_cell: number;
}

function axis(values: Array<string | null | undefined>, order: string[]): string[] {
  const seen = new Set(values.map((v) => (v && v.trim() ? v : UNLABELLED)));
  const known = order.filter((v) => seen.has(v));
  const extra = [...seen].filter((v) => !order.includes(v) && v !== UNLABELLED).sort();
  const out = [...known, ...extra];
  if (seen.has(UNLABELLED)) out.push(UNLABELLED);
  return out.length ? out : [UNLABELLED];
}

const label = (value: string | null | undefined): string => (value && value.trim() ? value : UNLABELLED);

/**
 * Cross the scenario library with the runs recorded against it. Scenario rows
 * describe planned coverage; run rows describe realised coverage. A cell with
 * scenarios but no accepted runs is "thin" — the variants exist but nothing has
 * been driven or rendered against them yet.
 */
export function buildCoverage(scenarios: ScenarioCoverageRow[], runs: RunCoverageRow[], minRunsPerCell = 3): CoverageMatrix {
  const visibility = axis([...scenarios.map((s) => s.visibility_class), ...runs.map((r) => r.visibility_class)], VISIBILITY_ORDER);
  const lighting = axis([...scenarios.map((s) => s.lighting_class), ...runs.map((r) => r.lighting_class)], LIGHTING_ORDER);

  const key = (v: string, l: string) => `${v}\u0000${l}`;
  const cells = new Map<string, CoverageCell>();
  for (const v of visibility) {
    for (const l of lighting) {
      cells.set(key(v, l), { visibility: v, lighting: l, scenarios: 0, reviewed: 0, runs: 0, accepted_runs: 0, duration_s: 0, distance_km: 0, status: "gap", odd: v !== UNLABELLED && l !== UNLABELLED });
    }
  }

  const routes = new Map<string, { route_class: string; scenarios: number; runs: number; accepted_runs: number; duration_s: number }>();
  const route = (name: string) => {
    let entry = routes.get(name);
    if (!entry) {
      entry = { route_class: name, scenarios: 0, runs: 0, accepted_runs: 0, duration_s: 0 };
      routes.set(name, entry);
    }
    return entry;
  };

  for (const s of scenarios) {
    const cell = cells.get(key(label(s.visibility_class), label(s.lighting_class)));
    const n = s.variants ?? 1;
    if (cell) {
      cell.scenarios += n;
      if (s.review_status === "reviewed") cell.reviewed += n;
    }
    route(label(s.route_class)).scenarios += n;
  }

  for (const r of runs) {
    const n = r.runs ?? 1;
    const accepted = r.quality_status === "accepted" ? n : 0;
    const cell = cells.get(key(label(r.visibility_class), label(r.lighting_class)));
    if (cell) {
      cell.runs += n;
      cell.accepted_runs += accepted;
      cell.duration_s += r.duration_s || 0;
      cell.distance_km += r.distance_km || 0;
    }
    const rc = route(label(r.route_class));
    rc.runs += n;
    rc.accepted_runs += accepted;
    rc.duration_s += r.duration_s || 0;
  }

  const list = [...cells.values()];
  for (const cell of list) {
    cell.duration_s = Math.round(cell.duration_s * 10) / 10;
    cell.distance_km = Math.round(cell.distance_km * 100) / 100;
    if (cell.accepted_runs >= minRunsPerCell) cell.status = "covered";
    else if (cell.scenarios > 0 || cell.runs > 0) cell.status = "thin";
    else cell.status = "gap";
  }

  const odd = list.filter((c) => c.odd);
  const covered = odd.filter((c) => c.status === "covered").length;
  const thin = odd.filter((c) => c.status === "thin").length;
  // Worst first: empty cells before thin ones, then the thinnest of those.
  const gaps = odd
    .filter((c) => c.status !== "covered")
    .sort((a, b) => a.accepted_runs - b.accepted_runs || a.scenarios - b.scenarios);

  return {
    axes: { visibility, lighting },
    cells: list,
    by_route_class: [...routes.values()].sort((a, b) => b.scenarios - a.scenarios || a.route_class.localeCompare(b.route_class)),
    gaps: gaps.map((c) => ({ visibility: c.visibility, lighting: c.lighting, status: c.status, scenarios: c.scenarios, runs: c.runs })),
    totals: {
      scenarios: list.reduce((n, c) => n + c.scenarios, 0),
      runs: list.reduce((n, c) => n + c.runs, 0),
      accepted_runs: list.reduce((n, c) => n + c.accepted_runs, 0),
      duration_s: Math.round(list.reduce((n, c) => n + c.duration_s, 0) * 10) / 10,
      distance_km: Math.round(list.reduce((n, c) => n + c.distance_km, 0) * 100) / 100,
      cells: odd.length,
      covered_cells: covered,
      thin_cells: thin,
      gap_cells: odd.length - covered - thin,
      coverage_pct: odd.length ? Math.round((covered / odd.length) * 1000) / 10 : 0,
    },
    min_runs_per_cell: minRunsPerCell,
  };
}
