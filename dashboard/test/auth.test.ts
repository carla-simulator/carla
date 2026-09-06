import { test } from "node:test";
import assert from "node:assert/strict";
import { parseTokens } from "../src/auth";

test("parseTokens reads tenant and role, defaults to reader, skips junk", () => {
  const map = parseTokens("a=default:writer; b=fleet-a ;;bad;=x;c=t:unknown");
  assert.deepEqual(map.get("a"), { tenant: "default", role: "writer" });
  assert.deepEqual(map.get("b"), { tenant: "fleet-a", role: "reader" });
  assert.equal(map.has("c"), false);
  assert.equal(map.size, 2);
  assert.equal(parseTokens(undefined).size, 0);
});
