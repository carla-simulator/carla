import { test } from "node:test";
import assert from "node:assert/strict";
import { DEFAULT_INPUT, GPU_CATALOG, WORKLOADS, coverageChecklist, datasetTier, estimateBudget, normaliseInput } from "../src/budget";

test("dataset tiers follow the rule of thumb", () => {
  assert.equal(datasetTier(500).id, "toy");
  assert.equal(datasetTier(1000).id, "proof_of_concept");
  assert.equal(datasetTier(9999).id, "proof_of_concept");
  assert.equal(datasetTier(10000).id, "domain_adaptation");
  assert.equal(datasetTier(100000).id, "robust");
  assert.equal(datasetTier(5e6).id, "robust");
});

test("a 100k-scene A100 fine-tune lands in the documented cost band", () => {
  const e = estimateBudget({ scenes: 100000, workload: "finetune", gpu: "a100_80gb" });
  assert.equal(e.tier.id, "robust");
  // 100-1000 A100 GPU-hours per 100k scenes plus 15% preprocessing.
  assert.equal(e.compute.a100_gpu_hours.low, 115);
  assert.equal(e.compute.a100_gpu_hours.high, 1150);
  assert.equal(e.compute.gpu_hours.mid, e.compute.a100_gpu_hours.mid);
  // Roughly $100-$1,600 training on A100s at $0.90-$1.40/h.
  assert.ok(e.cost.training_usd.low > 100 && e.cost.training_usd.low < 120, String(e.cost.training_usd.low));
  assert.ok(e.cost.training_usd.high > 1500 && e.cost.training_usd.high < 1700, String(e.cost.training_usd.high));
  assert.ok(e.cost.total_usd.mid >= e.cost.training_usd.mid);
  assert.ok(e.cost.total_usd.low <= e.cost.total_usd.mid && e.cost.total_usd.mid <= e.cost.total_usd.high);
  // 10 s x 20 fps x 720p video x 100k clips is in the "100 GB to multiple TB" band.
  assert.equal(e.storage.frames_per_scene, 200);
  assert.ok(e.storage.dataset_gb.low > 100 && e.storage.dataset_gb.high < 10000, JSON.stringify(e.storage.dataset_gb));
  assert.equal(e.storage.exceeds_default_disk, true);
  assert.ok(e.assumptions.length >= 4);
});

test("from-scratch runs scale with scene count and never fall below the floor", () => {
  const big = estimateBudget({ scenes: 200000, workload: "scratch" });
  const ref = estimateBudget({ scenes: 100000, workload: "scratch" });
  const tiny = estimateBudget({ scenes: 100, workload: "scratch" });
  assert.equal(big.compute.a100_gpu_hours.mid, ref.compute.a100_gpu_hours.mid * 2);
  assert.equal(ref.compute.a100_gpu_hours.high, WORKLOADS.scratch.a100_hours_per_100k.high * 1.15);
  assert.equal(tiny.compute.a100_gpu_hours.mid, ref.compute.a100_gpu_hours.mid * 0.05);
  assert.equal(tiny.tier.id, "toy");
});

test("faster GPUs need fewer hours, price overrides and interruptible discounts apply to every band", () => {
  const a100 = estimateBudget({ scenes: 50000, workload: "medium" });
  const h100 = estimateBudget({ scenes: 50000, workload: "medium", gpu: "h100_80gb" });
  assert.ok(h100.compute.gpu_hours.mid < a100.compute.gpu_hours.mid);
  assert.equal(h100.compute.gpu_hours.mid, Math.round((a100.compute.a100_gpu_hours.mid / GPU_CATALOG.h100_80gb.throughput) * 10) / 10);
  const quoted = estimateBudget({ scenes: 50000, workload: "medium", price_per_hour: 2, interruptible: true });
  assert.deepEqual(quoted.cost.price_per_hour, { low: 1.4, mid: 1.4, high: 1.4 });
  assert.equal(quoted.cost.training_usd.mid, Math.round(quoted.compute.gpu_hours.mid * 1.4 * 100) / 100);
  assert.ok(quoted.assumptions.some((a) => a.includes("overridden with $2/h")));
});

test("single images are sized as stills and wall-clock time follows parallelism", () => {
  const images = estimateBudget({ scenes: 20000, clip_seconds: 0, width: 1920, height: 1080 });
  assert.equal(images.storage.frames_per_scene, 1);
  // 1920x1080 at 2 bits/pixel ~ 0.52 MB, +10% metadata.
  assert.ok(images.storage.mb_per_scene.mid > 0.5 && images.storage.mb_per_scene.mid < 0.6, String(images.storage.mb_per_scene.mid));
  const one = estimateBudget({ scenes: 20000, parallel_gpus: 1 });
  const eight = estimateBudget({ scenes: 20000, parallel_gpus: 8 });
  assert.ok(Math.abs(one.compute.wall_clock_days.mid - eight.compute.wall_clock_days.mid * 8) < 0.05, `${one.compute.wall_clock_days.mid} vs ${eight.compute.wall_clock_days.mid}`);
  assert.ok(one.storage.rental_usd.mid > eight.storage.rental_usd.mid);
});

test("query-string input is coerced, bounded and defaulted", () => {
  const n = normaliseInput({ scenes: "2500", clip_seconds: "", fps: "abc", gpu: "tpu", workload: "scratch", interruptible: "true", parallel_gpus: "0", price_per_hour: "" });
  assert.equal(n.scenes, 2500);
  assert.equal(n.clip_seconds, DEFAULT_INPUT.clip_seconds);
  assert.equal(n.fps, DEFAULT_INPUT.fps);
  assert.equal(n.gpu, "a100_80gb");
  assert.equal(n.workload, "scratch");
  assert.equal(n.interruptible, true);
  assert.equal(n.parallel_gpus, 1);
  assert.equal(n.price_per_hour, null);
  assert.equal(normaliseInput({ scenes: 1e12 }).scenes, 1e8);
});

test("coverage checklist finds gaps on lighting, visibility, geometry and grade", () => {
  const rows = [
    { lighting_class: "daylight", visibility_class: "clear", route_class: "straight_flat", n: 40 },
    { lighting_class: "low_light", visibility_class: "heavy_rain", route_class: "curve_steep_descent", n: 10 },
    { lighting_class: "daylight", visibility_class: "fog", route_class: "hairpin_steep_climb", n: 2 },
  ];
  const c = coverageChecklist(rows);
  assert.equal(c.total, 52);
  const by = Object.fromEntries(c.axes.map((a) => [a.axis, a]));
  assert.deepEqual(by.lighting.missing, ["night"]);
  assert.deepEqual(by.visibility.missing, []);
  assert.equal(by.visibility.counts.rain, 10);
  assert.deepEqual(by.geometry.missing, []);
  assert.deepEqual(by.grade.missing, []);
  assert.equal(by.grade.balance, Math.round((2 / 52) * 1000) / 1000);
  assert.equal(by.lighting.balance, 0);
  assert.equal(c.gaps, 1);
  const empty = coverageChecklist([]);
  assert.equal(empty.total, 0);
  assert.equal(empty.gaps, 12);
});
