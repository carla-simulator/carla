/**
 * Dataset and compute planner.
 *
 * Turns a target synthetic/real driving corpus ("100k scenes at 10 s, 3 cameras")
 * into the three numbers a pilot budget actually needs: how much disk the corpus
 * takes, how many aggregate GPU-hours the training program costs, and what that
 * costs on a marketplace GPU (Vast.ai-style hourly pricing). Everything here is
 * pure arithmetic over published reference points so the Worker, the front end
 * and the tests all agree; the reference points themselves are listed in
 * `assumptions` on every response so a reader can argue with them.
 */

export type TierId = "proof_of_concept" | "domain_adaptation" | "robust";
export type ProgramId = "fine_tune" | "medium_train" | "from_scratch";
export type GpuId = "a100_80gb" | "h100_80gb" | "l40s" | "rtx4090";
export type ResolutionId = "720p" | "1080p" | "1440p" | "4k";

export interface Band {
  low: number;
  expected: number;
  high: number;
}

/** Dataset-size tiers: what a given scene count is actually good for. */
export const TIERS: Array<{ id: TierId; label: string; min_scenes: number; range: string; note: string }> = [
  { id: "proof_of_concept", label: "Proof of concept", min_scenes: 0, range: "1k–10k scenes", note: "Enough to demonstrate a pipeline end to end; not enough to claim robustness." },
  { id: "domain_adaptation", label: "Useful domain adaptation", min_scenes: 10_000, range: "10k–100k scenes", note: "Enough to move a pretrained model onto local roads, signage and traffic mix." },
  { id: "robust", label: "Robust across conditions", min_scenes: 100_000, range: "100k+ scenes", note: "The band where day/night, rain/fog, urban/highway/rural and rare events can all be covered." },
];

/**
 * Aggregate A100-class GPU-hours for a baseline corpus (BASELINE_FRAMES), before
 * scaling. The bands are wide on purpose: pipeline efficiency moves this more
 * than the model does.
 */
export const PROGRAMS: Record<ProgramId, { label: string; gpu_hours: Band; note: string }> = {
  fine_tune: { label: "Fine-tune / domain adaptation", gpu_hours: { low: 150, expected: 400, high: 1000 }, note: "Adapting an existing checkpoint; the cheapest way to use synthetic data." },
  medium_train: { label: "Medium training job", gpu_hours: { low: 1000, expected: 2000, high: 5000 }, note: "Retraining most of a perception or planning stack with a few sweeps." },
  from_scratch: { label: "From scratch / heavy iteration", gpu_hours: { low: 5000, expected: 10_000, high: 20_000 }, note: "Full training runs plus iteration; dominated by repeated sweeps, not the final run." },
};

/** Marketplace GPUs: throughput relative to an A100 80GB, and a typical $/hour. */
export const GPUS: Record<GpuId, { label: string; speed: number; usd_per_hour: number; memory_gb: number }> = {
  a100_80gb: { label: "A100 80GB", speed: 1, usd_per_hour: 1.15, memory_gb: 80 },
  h100_80gb: { label: "H100 80GB", speed: 2.2, usd_per_hour: 1.8, memory_gb: 80 },
  l40s: { label: "L40S 48GB", speed: 0.7, usd_per_hour: 0.8, memory_gb: 48 },
  rtx4090: { label: "RTX 4090 24GB", speed: 0.55, usd_per_hour: 0.35, memory_gb: 24 },
};

/** Encoded video bitrate per camera, in Mbit/s, for a driving-quality H.264/H.265 stream. */
export const RESOLUTIONS: Record<ResolutionId, { label: string; mbps: number }> = {
  "720p": { label: "1280×720", mbps: 4 },
  "1080p": { label: "1920×1080", mbps: 8 },
  "1440p": { label: "2560×1440", mbps: 14 },
  "4k": { label: "3840×2160", mbps: 28 },
};

/** 100k scenes × 10 s × 10 fps × 1 camera. Program hour bands are quoted at this size. */
export const BASELINE_FRAMES = 100_000 * 10 * 10;
/** Fixed pipeline overhead (preprocessing, validation, eval) makes cost sub-linear in frames. */
export const SCALE_EXPONENT = 0.85;
/** Interruptible instances trade availability for roughly a 40 % discount. */
export const INTERRUPTIBLE_DISCOUNT = 0.6;
/** Vast.ai instances default to a 10 GB disk; anything larger is billed per GB. */
export const DEFAULT_DISK_GB = 10;

export interface PlannerInput {
  scenes: number;
  clip_seconds: number;
  fps: number;
  cameras: number;
  resolution: ResolutionId;
  program: ProgramId;
  gpu: GpuId;
  gpus: number;
  interruptible: boolean;
  usd_per_gpu_hour: number | null;
  usd_per_gb_month: number;
  retention_months: number | null;
}

export const DEFAULT_INPUT: PlannerInput = {
  scenes: 100_000,
  clip_seconds: 10,
  fps: 10,
  cameras: 1,
  resolution: "1080p",
  program: "fine_tune",
  gpu: "a100_80gb",
  gpus: 1,
  interruptible: false,
  usd_per_gpu_hour: null,
  usd_per_gb_month: 0.15,
  retention_months: null,
};

export interface Plan {
  input: PlannerInput;
  tier: { id: TierId; label: string; range: string; note: string; next_tier_at: number | null };
  dataset: {
    scenes: number;
    frames: number;
    video_hours: number;
    raw_gb: number;
    labels_gb: number;
    working_gb: number;
    total_gb: number;
    extra_disk_gb: number;
    per_scene_mb: number;
  };
  compute: {
    gpu: string;
    gpus: number;
    usd_per_gpu_hour: number;
    interruptible: boolean;
    scale: number;
    gpu_hours: Band;
    wall_clock_days: Band;
    parallel_efficiency: number;
    cost_usd: Band;
  };
  storage: { months: number; usd_per_gb_month: number; cost_usd: number };
  total_usd: Band;
  assumptions: string[];
}

export interface Comparison {
  by_gpu: Array<{ id: GpuId; label: string; usd_per_gpu_hour: number; gpu_hours: number; cost_usd: number; wall_clock_days: number }>;
  by_program: Array<{ id: ProgramId; label: string; gpu_hours: number; cost_usd: number; note: string }>;
}

/** The same corpus priced on every GPU and for every training program, for the side-by-side view. */
export function comparePlans(input: PlannerInput): Comparison {
  return {
    by_gpu: (Object.keys(GPUS) as GpuId[]).map((id) => {
      const p = planProgram({ ...input, gpu: id, usd_per_gpu_hour: input.usd_per_gpu_hour });
      return { id, label: GPUS[id].label, usd_per_gpu_hour: p.compute.usd_per_gpu_hour, gpu_hours: p.compute.gpu_hours.expected, cost_usd: p.compute.cost_usd.expected, wall_clock_days: p.compute.wall_clock_days.expected };
    }),
    by_program: (Object.keys(PROGRAMS) as ProgramId[]).map((id) => {
      const p = planProgram({ ...input, program: id });
      return { id, label: PROGRAMS[id].label, gpu_hours: p.compute.gpu_hours.expected, cost_usd: p.compute.cost_usd.expected, note: PROGRAMS[id].note };
    }),
  };
}

function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}

function round(n: number, digits = 2): number {
  const f = Math.pow(10, digits);
  return Math.round(n * f) / f;
}

function mapBand(band: Band, f: (n: number) => number): Band {
  return { low: f(band.low), expected: f(band.expected), high: f(band.high) };
}

function pick<T extends string>(value: unknown, allowed: Record<T, unknown>, fallback: T): T {
  return typeof value === "string" && value in allowed ? (value as T) : fallback;
}

function num(value: unknown, fallback: number): number {
  const n = Number(value);
  return Number.isFinite(n) ? n : fallback;
}

/** Clamp a partial, untrusted input (query string or JSON body) into a usable plan input. */
export function normalizeInput(raw: Record<string, unknown>): PlannerInput {
  const d = DEFAULT_INPUT;
  const priceRaw = raw.usd_per_gpu_hour;
  const retentionRaw = raw.retention_months;
  return {
    scenes: Math.round(clamp(num(raw.scenes, d.scenes), 1, 100_000_000)),
    clip_seconds: round(clamp(num(raw.clip_seconds, d.clip_seconds), 0.1, 3600), 2),
    fps: round(clamp(num(raw.fps, d.fps), 1, 120), 2),
    cameras: Math.round(clamp(num(raw.cameras, d.cameras), 1, 32)),
    resolution: pick(raw.resolution, RESOLUTIONS, d.resolution),
    program: pick(raw.program, PROGRAMS, d.program),
    gpu: pick(raw.gpu, GPUS, d.gpu),
    gpus: Math.round(clamp(num(raw.gpus, d.gpus), 1, 512)),
    interruptible: raw.interruptible === true || raw.interruptible === "true" || raw.interruptible === "1",
    usd_per_gpu_hour: priceRaw === undefined || priceRaw === null || priceRaw === "" ? null : round(clamp(num(priceRaw, 0), 0.01, 100), 4),
    usd_per_gb_month: round(clamp(num(raw.usd_per_gb_month, d.usd_per_gb_month), 0, 10), 4),
    retention_months: retentionRaw === undefined || retentionRaw === null || retentionRaw === "" ? null : Math.round(clamp(num(retentionRaw, 1), 1, 120)),
  };
}

export function tierFor(scenes: number): { id: TierId; label: string; range: string; note: string; next_tier_at: number | null } {
  let index = 0;
  for (let i = 0; i < TIERS.length; i++) if (scenes >= TIERS[i].min_scenes) index = i;
  const tier = TIERS[index];
  const next = TIERS[index + 1];
  return { id: tier.id, label: tier.label, range: tier.range, note: tier.note, next_tier_at: next ? next.min_scenes : null };
}

/**
 * Parallel efficiency: an eighth GPU never gives an eighth of the wall clock.
 * 0.92 per doubling is a deliberately conservative data-parallel scaling factor.
 */
export function parallelEfficiency(gpus: number): number {
  return round(Math.pow(0.92, Math.log2(Math.max(1, gpus))), 4);
}

export function planProgram(rawInput: Record<string, unknown> | PlannerInput): Plan {
  const input = normalizeInput(rawInput as Record<string, unknown>);
  const gpu = GPUS[input.gpu];
  const program = PROGRAMS[input.program];
  const resolution = RESOLUTIONS[input.resolution];

  const frames = Math.round(input.scenes * input.clip_seconds * input.fps * input.cameras);
  const videoSeconds = input.scenes * input.clip_seconds;
  // Mbit/s → GB: (mbps × seconds × cameras) / 8 bits / 1000 MB-per-GB.
  const rawGb = (resolution.mbps * videoSeconds * input.cameras) / 8 / 1000;
  const labelsGb = rawGb * 0.08;                  // boxes, masks, poses, calibration, manifests
  const workingGb = (rawGb + labelsGb) * 1.0;     // decoded shards, caches, checkpoints during a run
  const totalGb = rawGb + labelsGb + workingGb;

  const scale = round(Math.pow(Math.max(frames / BASELINE_FRAMES, 1e-6), SCALE_EXPONENT), 4);
  const gpuHours = mapBand(program.gpu_hours, (h) => round((h * scale) / gpu.speed, 1));
  const efficiency = parallelEfficiency(input.gpus);
  const wallClockDays = mapBand(gpuHours, (h) => round(h / (input.gpus * efficiency) / 24, 2));
  const price = input.usd_per_gpu_hour ?? round(gpu.usd_per_hour * (input.interruptible ? INTERRUPTIBLE_DISCOUNT : 1), 4);
  const costUsd = mapBand(gpuHours, (h) => Math.round(h * price));

  const months = input.retention_months ?? Math.max(1, Math.ceil(wallClockDays.expected / 30));
  const storageCost = Math.round(totalGb * input.usd_per_gb_month * months);
  const totalUsd = mapBand(costUsd, (c) => c + storageCost);

  return {
    input,
    tier: tierFor(input.scenes),
    dataset: {
      scenes: input.scenes,
      frames,
      video_hours: round(videoSeconds / 3600, 1),
      raw_gb: round(rawGb, 1),
      labels_gb: round(labelsGb, 1),
      working_gb: round(workingGb, 1),
      total_gb: round(totalGb, 1),
      extra_disk_gb: round(Math.max(0, totalGb - DEFAULT_DISK_GB), 1),
      per_scene_mb: round((rawGb * 1000) / input.scenes, 2),
    },
    compute: {
      gpu: gpu.label,
      gpus: input.gpus,
      usd_per_gpu_hour: price,
      interruptible: input.interruptible,
      scale,
      gpu_hours: gpuHours,
      wall_clock_days: wallClockDays,
      parallel_efficiency: efficiency,
      cost_usd: costUsd,
    },
    storage: { months, usd_per_gb_month: input.usd_per_gb_month, cost_usd: storageCost },
    total_usd: totalUsd,
    assumptions: [
      `Dataset tiers: proof of concept 1k–10k scenes, useful domain adaptation 10k–100k, robust across weather/lighting/road types 100k+.`,
      `${program.label}: ${program.gpu_hours.low}–${program.gpu_hours.high} A100-class GPU-hours at ${(BASELINE_FRAMES / 1e6).toFixed(0)}M frames (100k scenes × 10 s × 10 fps × 1 camera), scaled by (frames/baseline)^${SCALE_EXPONENT} for fixed pipeline overhead.`,
      `${gpu.label} runs ${gpu.speed}× an A100 80GB for this workload and lists near $${gpu.usd_per_hour.toFixed(2)}/hour; marketplace prices move constantly with supply, so override the rate for a real quote.`,
      input.interruptible ? `Interruptible instances are priced at ${Math.round(INTERRUPTIBLE_DISCOUNT * 100)} % of on-demand — only safe with checkpointed training.` : `On-demand pricing; checkpointed pipelines can take roughly ${Math.round((1 - INTERRUPTIBLE_DISCOUNT) * 100)} % off on interruptible instances.`,
      `${resolution.label} at ${resolution.mbps} Mbit/s per camera; labels and manifests add 8 %, decoded shards and checkpoints add another copy during a run.`,
      `Instances default to a ${DEFAULT_DISK_GB} GB disk; this plan needs ${round(Math.max(0, totalGb - DEFAULT_DISK_GB), 1)} GB more, billed at $${input.usd_per_gb_month}/GB-month for ${months} month(s).`,
      `Coverage of day/night, rain/fog and urban/highway/rural matters more than raw count; for behaviour cloning, sequence length matters as much as frame count.`,
    ],
  };
}
