import { test } from "node:test";
import assert from "node:assert/strict";
import { BASELINE_FRAMES, GPUS, INTERRUPTIBLE_DISCOUNT, normalizeInput, parallelEfficiency, planProgram, tierFor } from "../src/planner";

test("scene count maps onto the dataset-size tiers", () => {
  assert.equal(tierFor(2_000).id, "proof_of_concept");
  assert.equal(tierFor(2_000).next_tier_at, 10_000);
  assert.equal(tierFor(10_000).id, "domain_adaptation");
  assert.equal(tierFor(99_999).id, "domain_adaptation");
  assert.equal(tierFor(100_000).id, "robust");
  assert.equal(tierFor(1_000_000).next_tier_at, null);
});

test("the baseline corpus reproduces the published A100 hour band", () => {
  const plan = planProgram({ scenes: 100_000, clip_seconds: 10, fps: 10, cameras: 1, program: "medium_train", gpu: "a100_80gb" });
  assert.equal(plan.dataset.frames, BASELINE_FRAMES);
  assert.equal(plan.compute.scale, 1);
  assert.deepEqual(plan.compute.gpu_hours, { low: 1000, expected: 2000, high: 5000 });
  // At the A100 list rate the cost band is the familiar $1k–$5k for a medium run.
  assert.equal(plan.compute.cost_usd.low, Math.round(1000 * GPUS.a100_80gb.usd_per_hour));
  assert.equal(plan.compute.cost_usd.high, Math.round(5000 * GPUS.a100_80gb.usd_per_hour));
  assert.ok(plan.compute.cost_usd.low < plan.compute.cost_usd.expected && plan.compute.cost_usd.expected < plan.compute.cost_usd.high);
});

test("storage follows resolution, clip length and camera count", () => {
  const plan = planProgram({ scenes: 100_000, clip_seconds: 10, fps: 10, cameras: 1, resolution: "1080p" });
  // 8 Mbit/s × 1e6 s / 8 / 1000 = 1000 GB raw, plus 8 % labels and one working copy.
  assert.equal(plan.dataset.raw_gb, 1000);
  assert.equal(plan.dataset.labels_gb, 80);
  assert.equal(plan.dataset.total_gb, 2160);
  assert.equal(plan.dataset.extra_disk_gb, 2150);
  assert.equal(plan.dataset.video_hours, 277.8);

  const threeCams = planProgram({ scenes: 100_000, clip_seconds: 10, fps: 10, cameras: 3, resolution: "1080p" });
  assert.equal(threeCams.dataset.raw_gb, 3000);        // one encoded stream per camera
  assert.equal(threeCams.dataset.frames, BASELINE_FRAMES * 3);

  const toy = planProgram({ scenes: 2_000, clip_seconds: 5, fps: 10, resolution: "720p" });
  assert.ok(toy.dataset.total_gb > 1 && toy.dataset.total_gb < 20, `toy demo should sit in the single-GB range, got ${toy.dataset.total_gb}`);
});

test("a faster GPU buys hours, not necessarily dollars", () => {
  const a100 = planProgram({ program: "fine_tune", gpu: "a100_80gb" });
  const h100 = planProgram({ program: "fine_tune", gpu: "h100_80gb" });
  assert.ok(h100.compute.gpu_hours.expected < a100.compute.gpu_hours.expected);
  assert.equal(h100.compute.gpu_hours.expected, Math.round((a100.compute.gpu_hours.expected / GPUS.h100_80gb.speed) * 10) / 10);
  assert.equal(h100.compute.usd_per_gpu_hour, GPUS.h100_80gb.usd_per_hour);
});

test("interruptible pricing and an explicit rate override the list price", () => {
  const onDemand = planProgram({ program: "fine_tune", gpu: "a100_80gb" });
  const spot = planProgram({ program: "fine_tune", gpu: "a100_80gb", interruptible: "true" });
  assert.equal(spot.compute.usd_per_gpu_hour, Math.round(GPUS.a100_80gb.usd_per_hour * INTERRUPTIBLE_DISCOUNT * 10000) / 10000);
  assert.ok(spot.compute.cost_usd.expected < onDemand.compute.cost_usd.expected);
  assert.ok(spot.assumptions.some((a) => a.includes("Interruptible instances are priced")));

  const quoted = planProgram({ program: "fine_tune", gpu: "h100_80gb", usd_per_gpu_hour: "0.90" });
  assert.equal(quoted.compute.usd_per_gpu_hour, 0.9);
  assert.equal(quoted.compute.cost_usd.expected, Math.round(quoted.compute.gpu_hours.expected * 0.9));
});

test("cost grows sub-linearly with frames and wall clock shrinks with GPUs", () => {
  const small = planProgram({ scenes: 10_000, program: "fine_tune" });
  const big = planProgram({ scenes: 100_000, program: "fine_tune" });
  const ratio = big.compute.gpu_hours.expected / small.compute.gpu_hours.expected;
  assert.ok(ratio > 1 && ratio < 10, `10x the scenes should cost more but less than 10x, got ${ratio}`);

  const one = planProgram({ scenes: 100_000, program: "medium_train", gpus: 1 });
  const eight = planProgram({ scenes: 100_000, program: "medium_train", gpus: 8 });
  assert.equal(eight.compute.gpu_hours.expected, one.compute.gpu_hours.expected);   // aggregate hours are unchanged
  assert.ok(eight.compute.wall_clock_days.expected < one.compute.wall_clock_days.expected / 4);
  assert.ok(eight.compute.wall_clock_days.expected > one.compute.wall_clock_days.expected / 8, "scaling is not perfectly linear");
  assert.equal(parallelEfficiency(1), 1);
  assert.ok(parallelEfficiency(8) < 1);
});

test("storage rent is added to every cost band", () => {
  const plan = planProgram({ scenes: 50_000, program: "fine_tune", retention_months: 6, usd_per_gb_month: 0.2 });
  assert.equal(plan.storage.months, 6);
  assert.equal(plan.storage.cost_usd, Math.round(plan.dataset.total_gb * 0.2 * 6));
  assert.equal(plan.total_usd.low, plan.compute.cost_usd.low + plan.storage.cost_usd);
  assert.equal(plan.total_usd.high, plan.compute.cost_usd.high + plan.storage.cost_usd);
});

test("untrusted query input is clamped instead of throwing", () => {
  const input = normalizeInput({ scenes: "-5", clip_seconds: "abc", fps: "1e9", cameras: "99", resolution: "8k", gpu: "tpu", program: "magic", gpus: "0", usd_per_gpu_hour: "", retention_months: "" });
  assert.equal(input.scenes, 1);
  assert.equal(input.clip_seconds, 10);           // fell back to the default
  assert.equal(input.fps, 120);                   // clamped
  assert.equal(input.cameras, 32);
  assert.equal(input.resolution, "1080p");
  assert.equal(input.gpu, "a100_80gb");
  assert.equal(input.program, "fine_tune");
  assert.equal(input.gpus, 1);
  assert.equal(input.usd_per_gpu_hour, null);
  assert.equal(input.retention_months, null);
  assert.equal(normalizeInput({}).scenes, 100_000);
});
