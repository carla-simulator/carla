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
