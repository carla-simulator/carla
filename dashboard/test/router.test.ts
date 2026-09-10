import { test } from "node:test";
import assert from "node:assert/strict";
import { Router } from "../src/router";

test("router matches multi-parameter paths and HEAD as GET", () => {
  const router = new Router()
    .get("/api/runs/:id/export/:format", () => new Response("export"))
    .get("/api/runs/:id", () => new Response("run"));
  const m = router.match("GET", "/api/runs/demo-run-01/export/mcap");
  assert.ok(m);
  assert.deepEqual(m.params, { id: "demo-run-01", format: "mcap" });
  assert.deepEqual(router.match("HEAD", "/api/runs/x")?.params, { id: "x" });
  assert.equal(router.match("POST", "/api/runs/x"), null);
  assert.equal(router.match("GET", "/api/runs/x/export"), null);
});

test("a literal path registered first wins over a parameter that would also match it", () => {
  // /api/plans/diff must not be read as a plan whose id is "diff".
  const router = new Router()
    .get("/api/plans/diff", () => new Response("diff"))
    .get("/api/plans/:id", () => new Response("plan"))
    .delete("/api/plans/:id", () => new Response("deleted"));
  assert.deepEqual(router.match("GET", "/api/plans/diff")?.params, {});
  assert.deepEqual(router.match("GET", "/api/plans/month-3")?.params, { id: "month-3" });
  // DELETE is a distinct method: the literal GET route does not shadow it.
  assert.deepEqual(router.match("DELETE", "/api/plans/diff")?.params, { id: "diff" });
});
