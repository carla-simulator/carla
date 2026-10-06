#!/usr/bin/env node
"use strict";
// `npm test`. Exercises the Node server in-process: the config layer's
// precedence and validation, CARLA detection, and the JSON-RPC shapes a client
// depends on. No CARLA and no network needed.
//
// Cross-language agreement with the Python server is checked separately, by
// tests/test_node_parity.py, which runs both over stdio and diffs the answers.

const assert = require("assert");
const fs = require("fs");
const os = require("os");
const path = require("path");

const REPO = path.join(__dirname, "..");
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), "cat-node-"));
process.env.CARLA_TOOLS_CONFIG = path.join(tmp, "config.env");
process.env.CARLA_SKILLS_DIR = path.join(REPO, "skills");
for (const k of ["CARLA_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT", "CARLA_UE4_ROOT",
  "CARLA_UE5_ROOT", "CARLA_UE58_ROOT", "SCENARIO_RUNNER_ROOT", "LEADERBOARD_ROOT",
  "SCENIC_ROOT", "PYTHON", "CARLA_UNREAL_ENGINE_PATH", "UE4_ROOT"]) delete process.env[k];

const cfg = require("../lib/config");
const skills = require("../lib/skills");
const server = require("../lib/server");

let passed = 0;
function test(name, fn) {
  try {
    fn();
    passed += 1;
  } catch (e) {
    console.error(`FAIL ${name}\n  ${e.message}`);
    process.exitCode = 1;
  }
}

// The structural markers detectCarla keys on for a 5.8 checkout.
function fakeUe58(root) {
  fs.mkdirSync(path.join(root, "Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Autoware"), { recursive: true });
  fs.mkdirSync(path.join(root, "CMake"), { recursive: true });
  fs.writeFileSync(path.join(root, "CMake/DLSS.cmake"), "");
  fs.writeFileSync(path.join(root, "CMakePresets.json"), "{}");
  fs.mkdirSync(path.join(root, "PythonAPI/carla"), { recursive: true });
  return root;
}

// --- config -----------------------------------------------------------------

test("a recorded value survives", () => {
  cfg.writeConfig({ SCENIC_ROOT: tmp });
  const r = cfg.resolve("SCENIC_ROOT");
  assert.strictEqual(r.value, tmp);
  assert.strictEqual(r.source, "user config");
});

test("an exported variable beats the config", () => {
  cfg.writeConfig({ SCENIC_ROOT: "/from/config" });
  process.env.SCENIC_ROOT = "/from/env";
  assert.deepStrictEqual(cfg.resolve("SCENIC_ROOT"), { value: "/from/env", source: "environment" });
  delete process.env.SCENIC_ROOT;
});

test("write merges, and an empty value clears", () => {
  cfg.writeConfig({ SCENIC_ROOT: "/a", LEADERBOARD_ROOT: "/b" });
  cfg.writeConfig({ SCENIC_ROOT: "" });
  const kept = cfg.readConfig();
  assert.ok(!("SCENIC_ROOT" in kept));
  assert.strictEqual(kept.LEADERBOARD_ROOT, "/b");
});

test("the config file cannot execute commands", () => {
  // Parsed, never sourced: a config file must not be able to run anything.
  fs.writeFileSync(process.env.CARLA_TOOLS_CONFIG, "SCENIC_ROOT=$(touch pwned)\n");
  assert.strictEqual(cfg.resolve("SCENIC_ROOT").value, "$(touch pwned)");
  assert.ok(!fs.existsSync(path.join(process.cwd(), "pwned")));
  cfg.writeConfig({ SCENIC_ROOT: "" });
});

// --- validation -------------------------------------------------------------

test("validation rejects the wrong kind of tree", () => {
  const sr = path.join(tmp, "scenario_runner");
  fs.mkdirSync(path.join(sr, "srunner"), { recursive: true });
  fs.writeFileSync(path.join(sr, "scenario_runner.py"), "");
  assert.ok(cfg.validate("SCENARIO_RUNNER_ROOT", sr).ok);
  const bad = cfg.validate("LEADERBOARD_ROOT", sr);
  assert.ok(!bad.ok);
  assert.ok(bad.why.includes("leaderboard_evaluator.py"), bad.why);
});

test("validation rejects missing and empty", () => {
  assert.ok(!cfg.validate("CARLA_ROOT", "").ok);
  const r = cfg.validate("CARLA_ROOT", path.join(tmp, "nope"));
  assert.ok(!r.ok && r.why.includes("not a directory"), r.why);
});

test("PYTHON is checked as a command, not a tree", () => {
  assert.ok(cfg.validate("PYTHON", "sh").ok, "a name on PATH is valid");
  assert.ok(!cfg.validate("PYTHON", tmp).ok, "a directory is not an interpreter");
});

// --- detection --------------------------------------------------------------

test("a 5.8 checkout is detected and derives its engine variable", () => {
  const info = cfg.detectCarla(fakeUe58(path.join(tmp, "carla")));
  assert.strictEqual(info.kind, "source");
  assert.strictEqual(info.flavor, "ue58");
  assert.strictEqual(info.vars.CARLA_UE58_ROOT, path.join(tmp, "carla"));
  assert.strictEqual(info.vars.CARLA_ROOT, path.join(tmp, "carla"));
});

test("5.5 is not mistaken for 5.8", () => {
  const root = path.join(tmp, "carla5");
  fs.mkdirSync(path.join(root, "Unreal/CarlaUnreal"), { recursive: true });
  fs.writeFileSync(path.join(root, "CMakePresets.json"), "{}");
  const info = cfg.detectCarla(root);
  assert.strictEqual(info.flavor, "ue5", "the Autoware/DLSS markers separate 5.8");
  assert.ok(!("CARLA_UE58_ROOT" in info.vars));
});

test("a release package is detected", () => {
  const root = path.join(tmp, "CARLA_0.9.16");
  fs.mkdirSync(path.join(root, "PythonAPI/carla"), { recursive: true });
  fs.writeFileSync(path.join(root, "CarlaUE4.sh"), "");
  const info = cfg.detectCarla(root);
  assert.strictEqual(info.kind, "release");
  assert.strictEqual(info.vars.CARLA_PACKAGE_ROOT, root);
  assert.strictEqual(info.vars.CARLA_TARGET, root);
});

test("a directory with no CARLA is rejected", () => {
  const info = cfg.detectCarla(path.join(tmp, "empty-dir-that-exists") === "" ? tmp : tmp);
  assert.strictEqual(info.kind, "none");
  assert.deepStrictEqual(info.vars, {});
});

// --- skills -----------------------------------------------------------------

test("the whole library is discovered", () => {
  const listed = skills.listSkills();
  assert.ok(listed.length >= 50, `only ${listed.length} skills found`);
  assert.ok(listed.every((e) => e.description), "a skill has no description");
});

test("read_skill anchors its output with an absolute directory", () => {
  const body = skills.readSkill("download-carla");
  const first = body.split("\n")[0];
  assert.ok(first.startsWith("Skill directory: /"), first);
  assert.ok(fs.existsSync(path.join(first.split(": ")[1], "SKILL.md")));
  assert.ok(body.includes("\n---\n"), "frontmatter missing after the anchor");
});

test("an unknown skill raises, it does not return empty", () => {
  assert.throws(() => skills.readSkill("no-such-skill"), /unknown skill/);
});

test("a bare machine gates the checkout groups but not python-api", () => {
  cfg.writeConfig({ SCENIC_ROOT: "", CARLA_ROOT: "", LEADERBOARD_ROOT: "" });
  const byName = new Map(skills.listSkills().map((e) => [e.name, e]));
  assert.ok(byName.get("spawn-vehicles").available, "a wheel-only skill must not be gated");
  // navigate-to imports `agents`, which ships only inside a CARLA tree.
  assert.ok(!byName.get("navigate-to").available);
  assert.ok(byName.get("navigate-to").unavailable_reason.includes("CARLA_ROOT"));
  assert.ok(byName.get("download-carla").available, "the way out of an empty machine");
  assert.ok(!byName.get("build-carla-ue58").available);
});

test("setting CARLA_ROOT derives the engine variable and opens its group", () => {
  const root = fakeUe58(path.join(tmp, "carla2"));
  const engine = path.join(tmp, "UnrealEngine");
  fs.mkdirSync(path.join(engine, "Engine/Build/BatchFiles"), { recursive: true });
  const out = skills.setConfig({ CARLA_ROOT: root, CARLA_UNREAL_ENGINE_PATH: engine });
  assert.ok(out.includes("also set CARLA_UE58_ROOT"), out);
  const ue58 = skills.listSkills("ue58");
  assert.ok(ue58.length > 0 && ue58.every((e) => e.available),
    JSON.stringify(ue58.find((e) => !e.available)));
});

test("set_config refuses an unusable path and stores nothing", () => {
  assert.throws(() => skills.setConfig({ CARLA_ROOT: tmp }), /PythonAPI\/carla/);
  assert.ok(!("CARLA_ROOT" in cfg.readConfig()) || cfg.readConfig().CARLA_ROOT !== tmp);
});

// --- the wire shapes a client depends on ------------------------------------

test("initialize echoes a known protocol version and names this package", () => {
  const r = server.dispatch({ method: "initialize", params: { protocolVersion: "2024-11-05" } });
  assert.strictEqual(r.protocolVersion, "2024-11-05", "a known version must be echoed");
  assert.strictEqual(r.serverInfo.name, "carla-agentic-tools");
  assert.strictEqual(r.serverInfo.version, require("../package.json").version);
  assert.ok(r.instructions.includes("set_config"), "the routing rule must ride the handshake");

  const unknown = server.dispatch({ method: "initialize", params: { protocolVersion: "1999-01-01" } });
  assert.strictEqual(unknown.protocolVersion, server.PROTOCOL_VERSIONS[0]);
});

test("tools/list advertises all five with schemas", () => {
  const names = server.dispatch({ method: "tools/list" }).tools.map((t) => t.name);
  assert.deepStrictEqual(names.sort(),
    ["check_prerequisites", "get_config", "list_skills", "read_skill", "set_config"]);
  for (const t of server.dispatch({ method: "tools/list" }).tools) {
    assert.strictEqual(t.inputSchema.type, "object", `${t.name} has no object schema`);
    assert.ok(t.description.length > 40, `${t.name} description too short to route on`);
  }
});

test("each return type takes the shape a client expects", () => {
  // A string and an array are wrapped in `result`; an object is not, because
  // only an object is valid at the top level of structuredContent.
  const s = server.dispatch({ method: "tools/call", params: { name: "read_skill", arguments: { name: "download-carla" } } });
  assert.strictEqual(s.content.length, 1);
  assert.strictEqual(s.isError, false);
  assert.ok(typeof s.structuredContent.result === "string");

  const a = server.dispatch({ method: "tools/call", params: { name: "list_skills", arguments: { group: "scenic" } } });
  assert.strictEqual(a.content.length, a.structuredContent.result.length);
  assert.ok(a.content.length >= 2);

  const o = server.dispatch({ method: "tools/call", params: { name: "get_config", arguments: {} } });
  assert.strictEqual(o.content.length, 1);
  assert.ok(!("result" in o.structuredContent), "an object must not be wrapped");
  assert.ok("config_file" in o.structuredContent && "keys" in o.structuredContent);
});

test("a failing tool is an isError result, not a transport error", () => {
  const r = server.dispatch({ method: "tools/call", params: { name: "read_skill", arguments: { name: "nope" } } });
  assert.strictEqual(r.isError, true);
  assert.ok(r.content[0].text.startsWith("Error executing tool read_skill:"), r.content[0].text);
});

test("a missing required argument fails the call, not the process", () => {
  const r = server.dispatch({ method: "tools/call", params: { name: "read_skill", arguments: {} } });
  assert.strictEqual(r.isError, true);
  assert.ok(r.content[0].text.includes("missing required argument 'name'"), r.content[0].text);
});

test("an unknown method is method-not-found, and ping answers", () => {
  assert.strictEqual(server.dispatch({ method: "nonsuch/thing" }), null);
  assert.deepStrictEqual(server.dispatch({ method: "ping" }), {});
});

// fs.rmSync arrived in 14.14; this suite also runs on the declared floor.
if (fs.rmSync) fs.rmSync(tmp, { recursive: true, force: true });
else fs.rmdirSync(tmp, { recursive: true });
console.log(`${passed} node checks passed on Node ${process.versions.node}`);
