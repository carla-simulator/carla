import { test } from "node:test";
import assert from "node:assert/strict";
import { planProgram } from "../src/planner";
import { comparePlanRecords, diffTotals, hydrate, planToCsv, planToMarkdown, slugPlanId, summarize, type SavedPlan } from "../src/plans";

function record(name: string, input: Record<string, unknown>, savedInput = input): SavedPlan {
  return hydrate({
    plan_id: slugPlanId(name),
    name,
    notes: null,
    input: planProgram(input).input,
    saved: summarize(planProgram(savedInput)),
    created_by: "tester",
    created_at: "2026-01-01T00:00:00Z",
    updated_at: null,
  });
}

test("plan ids are URL-safe slugs of the name", () => {
  assert.equal(slugPlanId("Month 3 — 100k scenes"), "month-3-100k-scenes");
  assert.equal(slugPlanId("  Fine-tune / A100  "), "fine-tune-a100");
  assert.match(slugPlanId("!!!"), /^plan-[a-z0-9]+$/);
  assert.ok(slugPlanId("x".repeat(200)).length <= 60);
});

test("a saved plan is repriced from its input, not from its stored totals", () => {
  // Saved when the plan was a 10k-scene fine-tune; the input now says 100k.
  const plan = record("month 3", { scenes: 100_000, program: "fine_tune" }, { scenes: 10_000, program: "fine_tune" });
  assert.equal(plan.current.scenes, 100_000);
  assert.equal(plan.saved.scenes, 10_000);
  assert.ok(plan.drift.length > 0, "the difference between the snapshot and today's numbers is reported");
  const scenes = plan.drift.find((d) => d.metric === "scenes");
  assert.deepEqual([scenes?.a, scenes?.b, scenes?.delta], [10_000, 100_000, 90_000]);
  assert.equal(scenes?.pct, 900);
});

test("an unchanged plan reports no drift", () => {
  const plan = record("steady", { scenes: 50_000, program: "medium_train" });
  assert.deepEqual(plan.drift, []);
  assert.deepEqual(plan.saved, plan.current);
});

test("diffing two plans names the inputs that moved and their cost effect", () => {
  const cheap = record("A100 on-demand", { scenes: 100_000, program: "fine_tune", gpu: "a100_80gb" });
  const spot = record("A100 interruptible", { scenes: 100_000, program: "fine_tune", gpu: "a100_80gb", interruptible: true });
  const diff = comparePlanRecords(cheap, spot);

  assert.deepEqual(diff.input.map((f) => f.field), ["interruptible"]);
  assert.deepEqual([diff.input[0].a, diff.input[0].b], [false, true]);
  const total = diff.metrics.find((m) => m.metric === "total_usd");
  assert.ok(total && total.delta < 0, "the interruptible plan is cheaper");
  // The corpus did not change, so no dataset metric shows up in the diff.
  assert.ok(!diff.metrics.some((m) => m.metric === "scenes" || m.metric === "total_gb"));
  assert.deepEqual([diff.a.plan_id, diff.b.plan_id], ["a100-on-demand", "a100-interruptible"]);
});

test("input diffs use labels, not raw enum ids", () => {
  const a = record("a", { gpu: "a100_80gb", program: "fine_tune", resolution: "1080p" });
  const b = record("b", { gpu: "h100_80gb", program: "from_scratch", resolution: "4k" });
  const byField = Object.fromEntries(comparePlanRecords(a, b).input.map((f) => [f.field, f]));
  assert.equal(byField.gpu.b, "H100 80GB");
  assert.equal(byField.program.b, "From scratch / heavy iteration");
  assert.equal(byField.resolution.b, "3840×2160");
});

test("diffTotals reports a percentage only where one is defined", () => {
  const base = summarize(planProgram({ scenes: 100_000 }));
  const same = diffTotals(base, { ...base });
  assert.deepEqual(same, []);
  const zeroed = diffTotals({ ...base, storage_usd: 0 }, { ...base, storage_usd: 500 });
  assert.equal(zeroed[0].pct, null);
  assert.equal(zeroed[0].delta, 500);
});

test("CSV export is a flat line-item budget", () => {
  const plan = record("month 3", { scenes: 100_000, clip_seconds: 10, fps: 10, program: "fine_tune", gpu: "a100_80gb" });
  const detail = planProgram(plan.input);
  const csv = planToCsv(plan, detail);
  const lines = csv.trim().split("\n");
  assert.equal(lines[0], "section,item,value,unit");
  assert.ok(lines.every((l) => l.split(",").length >= 4));
  const total = lines.find((l) => l.startsWith("cost,total (expected)"));
  assert.equal(total, `cost,total (expected),${detail.total_usd.expected},USD`);
  assert.ok(csv.includes(`dataset,total storage,${detail.dataset.total_gb},GB`));
});

test("CSV quotes cells that contain a comma", () => {
  const plan = record("Month 3, revised", { scenes: 1_000 });
  const csv = planToCsv(plan, planProgram(plan.input));
  assert.ok(csv.includes('plan,name,"Month 3, revised",'), csv.split("\n")[1]);
});

test("Markdown export carries the cost band and the assumptions", () => {
  const plan = record("month 3", { scenes: 100_000, program: "medium_train", gpu: "h100_80gb" });
  const detail = planProgram(plan.input);
  const md = planToMarkdown(plan, detail);
  assert.ok(md.startsWith("# Budget — month 3"));
  assert.ok(md.includes("| **Total** |"));
  assert.ok(md.includes("H100 80GB"));
  for (const assumption of detail.assumptions) assert.ok(md.includes("* " + assumption), "assumption missing: " + assumption);
  assert.ok(!/\n\n\n/.test(md), "no runs of blank lines");
});
