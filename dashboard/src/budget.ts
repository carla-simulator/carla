/**
 * Synthetic-data program budget planner.
 *
 * Turns "we want N driving scenes" into a low / mid / high estimate of
 * dataset tier, storage, GPU-hours and marketplace cost, plus a coverage
 * checklist against the scenario library. All coefficients are documented
 * here so a partner can audit the arithmetic; nothing is fetched at request
 * time. Prices are spot-market reference points for Vast.ai-style GPU
 * marketplaces (per-second billing, dynamic pricing) and are meant to be
 * overridden with a live quote via `price_per_hour`.
 */

export type Workload = "finetune" | "medium" | "scratch";
export type GpuModel = "a100_80gb" | "h100_80gb" | "l40s_48gb" | "rtx4090_24gb";
export type Band = "low" | "mid" | "high";

export interface BudgetInput {
  /** Number of scenes (clips or images) in the target dataset. */
  scenes: number;
  /** Seconds per clip; 0 means single images. */
  clip_seconds: number;
  fps: number;
  cameras: number;
  /** Frame resolution, e.g. 1280x720. */
  width: number;
  height: number;
  workload: Workload;
  gpu: GpuModel;
  /** Override the catalog price (USD per GPU-hour). Applies to all bands. */
  price_per_hour: number | null;
  /** Interruptible / spot instances: typically cheaper, needs checkpointing. */
  interruptible: boolean;
  /** GPUs used in parallel; drives wall-clock time and storage rental. */
  parallel_gpus: number;
  /** Instance disk price, USD per GB-month. */
  storage_price_gb_month: number;
}

export interface Triple { low: number; mid: number; high: number }

export interface BudgetEstimate {
  input: BudgetInput;
  tier: { id: string; label: string; range: string; note: string };
  storage: {
    frames_per_scene: number;
    mb_per_scene: Triple;
    dataset_gb: Triple;
    /** Vast.ai-style instances ship with ~10 GB; more disk raises the hourly rate. */
    exceeds_default_disk: boolean;
    rental_usd: Triple;
  };
  compute: {
    workload: { id: Workload; label: string };
    a100_gpu_hours: Triple;
    gpu_hours: Triple;
    wall_clock_days: Triple;
    preprocessing_share: number;
  };
  cost: {
    gpu: { id: GpuModel; label: string; memory_gb: number };
    price_per_hour: Triple;
    training_usd: Triple;
    total_usd: Triple;
    usd_per_scene: Triple;
  };
  assumptions: string[];
  recommendations: string[];
}

export const GPU_CATALOG: Record<GpuModel, { label: string; memory_gb: number; architecture: string; price: Triple; /** training throughput relative to A100 80GB */ throughput: number }> = {
  a100_80gb: { label: "A100 80 GB", memory_gb: 80, architecture: "Ampere", price: { low: 0.9, mid: 1.15, high: 1.4 }, throughput: 1.0 },
  h100_80gb: { label: "H100 80 GB", memory_gb: 80, architecture: "Hopper", price: { low: 0.9, mid: 1.8, high: 2.5 }, throughput: 2.2 },
  l40s_48gb: { label: "L40S 48 GB", memory_gb: 48, architecture: "Ada Lovelace", price: { low: 0.6, mid: 0.95, high: 1.3 }, throughput: 0.8 },
  rtx4090_24gb: { label: "RTX 4090 24 GB", memory_gb: 24, architecture: "Ada Lovelace", price: { low: 0.25, mid: 0.4, high: 0.6 }, throughput: 0.55 },
};

/** A100-equivalent GPU-hours for a 100k-scene program, by workload. */
export const WORKLOADS: Record<Workload, { label: string; description: string; a100_hours_per_100k: Triple }> = {
  finetune: { label: "Fine-tune / domain adaptation", description: "Adapt a pretrained perception or driving model to the local domain.", a100_hours_per_100k: { low: 100, mid: 400, high: 1000 } },
  medium: { label: "Medium training job", description: "Train a mid-size model with several epochs and a few ablations.", a100_hours_per_100k: { low: 1000, mid: 2500, high: 5000 } },
  scratch: { label: "From scratch / heavy iteration", description: "Full training runs with hyper-parameter sweeps and temporal or multi-view supervision.", a100_hours_per_100k: { low: 5000, mid: 10000, high: 20000 } },
};

export const DATASET_TIERS = [
  { id: "toy", label: "Toy demo", min: 0, max: 1000, range: "< 1k", note: "Enough to exercise a pipeline end to end, not to draw conclusions." },
  { id: "proof_of_concept", label: "Proof of concept", min: 1000, max: 10000, range: "1k-10k", note: "Shows a model can learn the domain; expect brittle behaviour outside the captured conditions." },
  { id: "domain_adaptation", label: "Useful domain adaptation", min: 10000, max: 100000, range: "10k-100k", note: "Meaningful fine-tuning of a pretrained model for a corridor or ODD." },
  { id: "robust", label: "Robust across weather, lighting and road types", min: 100000, max: Infinity, range: "100k+", note: "Coverage of rare events and edge cases becomes the limiting factor, not count." },
] as const;

/** Compressed video bits per pixel per frame (H.264/H.265 at driving-recorder quality). */
const BITS_PER_PIXEL: Triple = { low: 0.08, mid: 0.15, high: 0.3 };
/** Single JPEG/PNG images compress worse than video; bits per pixel per image. */
const IMAGE_BITS_PER_PIXEL: Triple = { low: 1.0, mid: 2.0, high: 4.0 };
/** Labels, telemetry, manifests and indexes as a share of media bytes. */
const METADATA_OVERHEAD = 0.1;
/** GPU time spent before training: decoding, augmentation, validation and dataset checks. */
const PREPROCESSING_SHARE = 0.15;
/** Small datasets still cost fixed iteration time; never scale below this share of the 100k figure. */
const MIN_SCALE = 0.05;
const INTERRUPTIBLE_DISCOUNT = 0.7;
const DEFAULT_DISK_GB = 10;

export const DEFAULT_INPUT: BudgetInput = {
  scenes: 100000, clip_seconds: 10, fps: 20, cameras: 1, width: 1280, height: 720,
  workload: "finetune", gpu: "a100_80gb", price_per_hour: null, interruptible: false, parallel_gpus: 8, storage_price_gb_month: 0.15,
};

const round = (v: number, d = 0): number => { const f = Math.pow(10, d); return Math.round(v * f) / f; };
const mapTriple = (t: Triple, f: (v: number, band: Band) => number): Triple => ({ low: f(t.low, "low"), mid: f(t.mid, "mid"), high: f(t.high, "high") });

export function datasetTier(scenes: number): (typeof DATASET_TIERS)[number] {
  return DATASET_TIERS.find((t) => scenes >= t.min && scenes < t.max) || DATASET_TIERS[DATASET_TIERS.length - 1];
}

/** Coerce and bound user-supplied values; unknown enums fall back to defaults. */
export function normaliseInput(raw: Partial<Record<keyof BudgetInput, unknown>>): BudgetInput {
  const num = (v: unknown, fallback: number, min: number, max: number): number => {
    const n = typeof v === "string" && v.trim() === "" ? NaN : Number(v);
    return Number.isFinite(n) ? Math.min(max, Math.max(min, n)) : fallback;
  };
  const bool = (v: unknown, fallback: boolean): boolean => (v === undefined || v === null || v === "" ? fallback : v === true || v === "true" || v === "1" || v === 1);
  const d = DEFAULT_INPUT;
  const workload = typeof raw.workload === "string" && raw.workload in WORKLOADS ? (raw.workload as Workload) : d.workload;
  const gpu = typeof raw.gpu === "string" && raw.gpu in GPU_CATALOG ? (raw.gpu as GpuModel) : d.gpu;
  const price = raw.price_per_hour === undefined || raw.price_per_hour === null || raw.price_per_hour === "" ? null : num(raw.price_per_hour, NaN, 0.01, 1000);
  return {
    scenes: Math.round(num(raw.scenes, d.scenes, 1, 1e8)),
    clip_seconds: num(raw.clip_seconds, d.clip_seconds, 0, 3600),
    fps: num(raw.fps, d.fps, 1, 120),
    cameras: Math.round(num(raw.cameras, d.cameras, 1, 16)),
    width: Math.round(num(raw.width, d.width, 64, 8192)),
    height: Math.round(num(raw.height, d.height, 64, 8192)),
    workload,
    gpu,
    price_per_hour: price !== null && Number.isFinite(price) ? price : null,
    interruptible: bool(raw.interruptible, d.interruptible),
    parallel_gpus: Math.round(num(raw.parallel_gpus, d.parallel_gpus, 1, 1024)),
    storage_price_gb_month: num(raw.storage_price_gb_month, d.storage_price_gb_month, 0, 100),
  };
}

export function estimateBudget(partial: Partial<BudgetInput> = {}): BudgetEstimate {
  const input = normaliseInput(partial);
  const tier = datasetTier(input.scenes);
  const workload = WORKLOADS[input.workload];
  const gpu = GPU_CATALOG[input.gpu];

  // ---- storage
  const isVideo = input.clip_seconds > 0;
  const framesPerScene = isVideo ? Math.max(1, Math.round(input.clip_seconds * input.fps)) : 1;
  const pixels = input.width * input.height;
  const bpp = isVideo ? BITS_PER_PIXEL : IMAGE_BITS_PER_PIXEL;
  const mbPerScene = mapTriple(bpp, (b) => ((pixels * b) / 8 / 1e6) * framesPerScene * input.cameras * (1 + METADATA_OVERHEAD));
  const datasetGb = mapTriple(mbPerScene, (mb) => (mb * input.scenes) / 1000);

  // ---- compute
  const scale = Math.max(MIN_SCALE, input.scenes / 100000);
  const a100Hours = mapTriple(workload.a100_hours_per_100k, (h) => h * scale * (1 + PREPROCESSING_SHARE));
  const gpuHours = mapTriple(a100Hours, (h) => h / gpu.throughput);
  const wallClockDays = mapTriple(gpuHours, (h) => h / input.parallel_gpus / 24);

  // ---- cost
  const discount = input.interruptible ? INTERRUPTIBLE_DISCOUNT : 1;
  const price = mapTriple(gpu.price, (p) => (input.price_per_hour ?? p) * discount);
  const trainingUsd = mapTriple(gpuHours, (h, band) => h * price[band]);
  // Disk is rented for the wall-clock duration on every parallel instance (each needs the dataset or a shard of it).
  const rentalUsd = mapTriple(datasetGb, (gb, band) => gb * input.storage_price_gb_month * Math.max(1 / 30, wallClockDays[band] / 30));
  const totalUsd = mapTriple(trainingUsd, (usd, band) => usd + rentalUsd[band]);

  const assumptions = [
    `${WORKLOADS[input.workload].label}: ${workload.a100_hours_per_100k.low}-${workload.a100_hours_per_100k.high} A100 GPU-hours per 100k scenes, scaled linearly with a ${MIN_SCALE * 100}% floor, plus ${PREPROCESSING_SHARE * 100}% for preprocessing, augmentation and validation.`,
    `${gpu.label} (${gpu.architecture}) at ${gpu.throughput}x A100 training throughput; reference marketplace price $${gpu.price.low}-$${gpu.price.high}/h${input.price_per_hour !== null ? `, overridden with $${input.price_per_hour}/h` : ""}${input.interruptible ? `, interruptible discount ${Math.round((1 - INTERRUPTIBLE_DISCOUNT) * 100)}%` : ""}.`,
    isVideo
      ? `${input.clip_seconds} s clips at ${input.fps} fps, ${input.cameras} camera(s), ${input.width}x${input.height}, ${BITS_PER_PIXEL.low}-${BITS_PER_PIXEL.high} bits/pixel compressed video, +${METADATA_OVERHEAD * 100}% labels and telemetry.`
      : `Single images, ${input.cameras} camera(s), ${input.width}x${input.height}, ${IMAGE_BITS_PER_PIXEL.low}-${IMAGE_BITS_PER_PIXEL.high} bits/pixel, +${METADATA_OVERHEAD * 100}% labels and telemetry.`,
    `Instance disk at $${input.storage_price_gb_month}/GB-month on ${input.parallel_gpus} parallel GPU(s) for the wall-clock duration; long-term object storage (R2/S3) is not included.`,
  ];

  const recommendations: string[] = [];
  if (input.gpu !== "a100_80gb" && input.workload !== "scratch") recommendations.push("A100 80 GB usually gives the best cost-to-capability ratio for a first run; switch to H100 once the pipeline is stable and throughput-bound.");
  if (!input.interruptible) recommendations.push("Once training is checkpointed, interruptible instances cut the GPU bill by roughly a third.");
  if (datasetGb.mid > DEFAULT_DISK_GB) recommendations.push(`The dataset exceeds the ~${DEFAULT_DISK_GB} GB default instance disk; request more disk when renting (raises the hourly rate) or stream shards from object storage.`);
  if (tier.id === "robust") recommendations.push("Above 100k scenes, coverage of night, rain, fog, road types and rare events matters more than raw count; use the coverage checklist to direct synthetic generation.");
  if (tier.id === "toy" || tier.id === "proof_of_concept") recommendations.push("Below 10k scenes, prefer fine-tuning a pretrained model over training from scratch.");
  if (input.workload === "scratch") recommendations.push("Supplementing synthetic scenes with real captures typically reduces total compute compared with pure from-scratch training.");
  if (isVideo && input.clip_seconds < 5) recommendations.push("For behaviour cloning or end-to-end driving, sequence length matters as much as frame count; clips under 5 s rarely capture a manoeuvre.");

  return {
    input,
    tier: { id: tier.id, label: tier.label, range: tier.range, note: tier.note },
    storage: {
      frames_per_scene: framesPerScene,
      mb_per_scene: mapTriple(mbPerScene, (v) => round(v, 3)),
      dataset_gb: mapTriple(datasetGb, (v) => round(v, 1)),
      exceeds_default_disk: datasetGb.mid > DEFAULT_DISK_GB,
      rental_usd: mapTriple(rentalUsd, (v) => round(v, 2)),
    },
    compute: {
      workload: { id: input.workload, label: workload.label },
      a100_gpu_hours: mapTriple(a100Hours, (v) => round(v, 1)),
      gpu_hours: mapTriple(gpuHours, (v) => round(v, 1)),
      wall_clock_days: mapTriple(wallClockDays, (v) => round(v, 2)),
      preprocessing_share: PREPROCESSING_SHARE,
    },
    cost: {
      gpu: { id: input.gpu, label: gpu.label, memory_gb: gpu.memory_gb },
      price_per_hour: mapTriple(price, (v) => round(v, 3)),
      training_usd: mapTriple(trainingUsd, (v) => round(v, 2)),
      total_usd: mapTriple(totalUsd, (v) => round(v, 2)),
      usd_per_scene: mapTriple(totalUsd, (v) => round(v / input.scenes, 4)),
    },
    assumptions,
    recommendations,
  };
}

// ---------------------------------------------------------------- coverage

export interface CoverageAxis {
  axis: string;
  label: string;
  /** Values every robust dataset needs on this axis. */
  required: string[];
  counts: Record<string, number>;
  missing: string[];
  /** Share of scenes on the rarest required value; 0 when something is missing. */
  balance: number;
}

export interface CoverageRow { lighting_class: string | null; visibility_class: string | null; route_class: string | null; n: number }

const GEOMETRY = ["straight", "curve", "hairpin"];
const GRADE = ["flat", "climb", "descent"];

function routeParts(routeClass: string | null): { geometry: string; grade: string } {
  const rc = (routeClass || "unknown").toLowerCase();
  return {
    geometry: GEOMETRY.find((g) => rc.startsWith(g)) || "unknown",
    grade: GRADE.find((g) => rc.endsWith(g)) || "unknown",
  };
}

/**
 * Build the coverage checklist from per-scenario class counts. Axes follow the
 * "what matters most" list: lighting (day/night), visibility (rain/fog),
 * road geometry and grade stand in for urban/highway/rural on mountain corridors.
 */
export function coverageChecklist(rows: CoverageRow[]): { total: number; axes: CoverageAxis[]; gaps: number } {
  const total = rows.reduce((n, r) => n + r.n, 0);
  const spec: Array<{ axis: string; label: string; required: string[]; pick: (r: CoverageRow) => string }> = [
    { axis: "lighting", label: "Lighting", required: ["daylight", "low_light", "night"], pick: (r) => r.lighting_class || "unknown" },
    { axis: "visibility", label: "Visibility / weather", required: ["clear", "rain", "fog"], pick: (r) => r.visibility_class || "unknown" },
    { axis: "geometry", label: "Road geometry", required: GEOMETRY, pick: (r) => routeParts(r.route_class).geometry },
    { axis: "grade", label: "Road grade", required: GRADE, pick: (r) => routeParts(r.route_class).grade },
  ];
  const axes = spec.map(({ axis, label, required, pick }) => {
    const counts: Record<string, number> = {};
    for (const r of rows) {
      const key = pick(r);
      counts[key] = (counts[key] || 0) + r.n;
    }
    // heavy_rain counts toward rain coverage.
    if (axis === "visibility" && counts.heavy_rain) counts.rain = (counts.rain || 0) + counts.heavy_rain;
    const missing = required.filter((v) => !(counts[v] > 0));
    const balance = missing.length || !total ? 0 : round(Math.min(...required.map((v) => counts[v])) / total, 3);
    return { axis, label, required, counts, missing, balance };
  });
  return { total, axes, gaps: axes.reduce((n, a) => n + a.missing.length, 0) };
}
