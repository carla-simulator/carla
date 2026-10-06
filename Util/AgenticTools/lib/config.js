"use strict";
// Persisted paths for the skill library — the Node half.
//
// A faithful port of src/carla_agentic_tools/config.py: same file, same
// precedence, same structural markers, same validators. Both servers read and
// write one config, so a path recorded through either is visible to the other,
// and tests/test_node_parity.py fails if the two drift.
//
// Resolution order for every key, highest first:
//   1. an explicit environment variable   one-off override, CI
//   2. ./.carla-tools.env                 a repo carrying its own CARLA
//   3. the user config                    the normal case
//   4. each env.sh's own search list      last resort

const fs = require("fs");
const os = require("os");
const path = require("path");
const { spawnSync } = require("child_process");

// Keys a user is ever asked about. CARLA_ROOT is the only CARLA path anyone is
// prompted for; the derived keys below it come from detectCarla.
const CONFIG_KEYS = {
  CARLA_ROOT: "a CARLA release or source checkout (holds PythonAPI/carla)",
  PYTHON: "the interpreter that imports the carla wheel",
  SCENARIO_RUNNER_ROOT: "a scenario_runner checkout",
  LEADERBOARD_ROOT: "a leaderboard checkout",
  SCENIC_ROOT: "a Scenic checkout or installed package",
  CARLA_UNREAL_ENGINE_PATH: "the Unreal Engine fork CARLA builds against",
  UE4_ROOT: "the built CarlaUnreal UE 4.26 fork, for the ue4 editor",
};

const DERIVED_KEYS = [
  "CARLA_UE4_ROOT", "CARLA_UE5_ROOT", "CARLA_UE58_ROOT",
  "CARLA_PACKAGE_ROOT", "CARLA_TARGET",
];

const PROJECT_CONFIG = ".carla-tools.env";
const LINE = /^\s*(?:export\s+)?([A-Z][A-Z0-9_]*)\s*=\s*(.*?)\s*$/;

function userConfigPath() {
  if (process.env.CARLA_TOOLS_CONFIG) return process.env.CARLA_TOOLS_CONFIG;
  const base = process.env.XDG_CONFIG_HOME || path.join(os.homedir(), ".config");
  return path.join(base, "carla-agentic-tools", "config.env");
}

// Parsed, never executed: a config file must not be able to run commands, and
// values are taken literally with no expansion.
function parseFile(file) {
  const out = {};
  let text;
  try {
    text = fs.readFileSync(file, "utf8");
  } catch (e) {
    return out;
  }
  for (const line of text.split("\n")) {
    if (!line.trim() || line.trimStart().startsWith("#")) continue;
    const m = LINE.exec(line);
    if (m) out[m[1]] = m[2].replace(/^['"]|['"]$/g, "");
  }
  return out;
}

function readConfig(cwd) {
  const merged = parseFile(userConfigPath());
  return Object.assign(merged, parseFile(path.join(cwd || process.cwd(), PROJECT_CONFIG)));
}

function resolve(key, cwd) {
  if (process.env[key]) return { value: process.env[key], source: "environment" };
  const cfg = readConfig(cwd);
  if (cfg[key]) {
    const project = parseFile(path.join(cwd || process.cwd(), PROJECT_CONFIG));
    return { value: cfg[key], source: key in project ? "project config" : "user config" };
  }
  return { value: "", source: "unset" };
}

function writeConfig(updates, file) {
  const target = file || userConfigPath();
  fs.mkdirSync(path.dirname(target), { recursive: true });
  const merged = parseFile(target);
  for (const [k, v] of Object.entries(updates)) {
    if (v) merged[k] = String(v);
    else delete merged[k]; // an empty value clears a key
  }
  const body = Object.keys(merged).sort().map((k) => `${k}=${merged[k]}`).join("\n");
  fs.writeFileSync(target,
    "# Written by carla-agentic-tools. An exported environment variable of\n" +
    "# the same name still wins over anything here.\n" +
    body + "\n");
  return target;
}

// --- validation ------------------------------------------------------------
// A key that merely has a value is not configured: a path can be deleted,
// renamed, or point at the wrong kind of tree. Each marker is the one the
// matching carla_*_is_root helper in the skills' env.sh tests for.

const isDir = (p) => { try { return fs.statSync(p).isDirectory(); } catch (e) { return false; } };
const isFile = (p) => { try { return fs.statSync(p).isFile(); } catch (e) { return false; } };
const has = (root, ...rel) => rel.some((r) => fs.existsSync(path.join(root, r)));

const VALIDATORS = {
  CARLA_ROOT: [(p) => isDir(path.join(p, "PythonAPI/carla")),
    "a CARLA root holding PythonAPI/carla"],
  SCENARIO_RUNNER_ROOT: [(p) => isFile(path.join(p, "scenario_runner.py")) && isDir(path.join(p, "srunner")),
    "a scenario_runner checkout (scenario_runner.py + srunner/)"],
  LEADERBOARD_ROOT: [(p) => isFile(path.join(p, "leaderboard/leaderboard_evaluator.py")),
    "a leaderboard checkout (leaderboard/leaderboard_evaluator.py)"],
  SCENIC_ROOT: [(p) => has(p, "src/scenic", "examples/carla", "simulators/carla"),
    "a Scenic checkout or the installed scenic package"],
  CARLA_UE4_ROOT: [(p) => isFile(path.join(p, "Unreal/CarlaUE4/CarlaUE4.uproject")),
    "a ue4-dev checkout"],
  CARLA_UE5_ROOT: [(p) => isFile(path.join(p, "CMakePresets.json")), "a ue5-dev checkout"],
  CARLA_UE58_ROOT: [(p) => isFile(path.join(p, "CMakePresets.json")), "a ue58-dev checkout"],
  CARLA_UNREAL_ENGINE_PATH: [(p) => has(p, "Engine/Build/BatchFiles", "Setup.sh"),
    "an Unreal Engine source tree"],
  UE4_ROOT: [(p) => has(p, "Engine/Build/BatchFiles", "Setup.sh"),
    "an Unreal Engine 4.26 source tree"],
};

function expandUser(p) {
  if (p === "~") return os.homedir();
  if (p.startsWith("~/")) return path.join(os.homedir(), p.slice(2));
  return p;
}

// PYTHON is a command rather than a tree, so it is checked for being runnable;
// whether it imports `carla` is left to the skill's check_env.sh, which can say
// so precisely instead of guessing from a path.
function validate(key, value) {
  if (!value) return { ok: false, why: "unset" };
  if (key === "PYTHON") {
    const p = expandUser(value);
    if (isFile(p)) {
      try { fs.accessSync(p, fs.constants.X_OK); return { ok: true, why: "" }; } catch (e) { /* fall through */ }
    }
    const probe = spawnSync(process.platform === "win32" ? "where" : "which", [value],
      { stdio: "ignore" });
    if (!probe.error && probe.status === 0) return { ok: true, why: "" };
    return { ok: false, why: `${value} is not an executable on PATH` };
  }
  const p = expandUser(value);
  if (!isDir(p)) return { ok: false, why: `${p} is not a directory` };
  const check = VALIDATORS[key];
  if (check && !check[0](p)) return { ok: false, why: `${p} is not ${check[1]}` };
  return { ok: true, why: "" };
}

function resolveValid(key, cwd) {
  const { value, source } = resolve(key, cwd);
  const { ok, why } = validate(key, value);
  return { value, source, problem: ok ? "" : why };
}

// --- CARLA detection -------------------------------------------------------

function branchOf(p) {
  try {
    const out = spawnSync("git", ["-C", p, "rev-parse", "--abbrev-ref", "HEAD"],
      { encoding: "utf8", timeout: 10000 });
    return out.status === 0 ? out.stdout.trim() : "";
  } catch (e) {
    return "";
  }
}

// Classify a directory and say which variables it sets. The flavor is what gates
// the matching skill group, so it is derived here rather than inferred from
// whichever variable name the user happened to set.
function detectCarla(input) {
  const p = expandUser(String(input));
  const info = { path: p, kind: "none", flavor: "", detail: "", vars: {} };
  if (!isDir(p)) {
    info.detail = "not a directory";
    return info;
  }
  const hasApi = isDir(path.join(p, "PythonAPI/carla"));

  if (isFile(path.join(p, "CMakePresets.json")) && isDir(path.join(p, "Unreal/CarlaUnreal"))) {
    // ue5-dev and ue58-dev share this shape; the Autoware plugin and the DLSS
    // CMake module arrived with 5.8 and are what tell them apart.
    const is58 = isDir(path.join(p, "Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Autoware"))
      && isFile(path.join(p, "CMake/DLSS.cmake"));
    info.kind = "source";
    info.flavor = is58 ? "ue58" : "ue5";
    info.vars[is58 ? "CARLA_UE58_ROOT" : "CARLA_UE5_ROOT"] = p;
  } else if (isFile(path.join(p, "Unreal/CarlaUE4/CarlaUE4.uproject"))) {
    info.kind = "source";
    info.flavor = "ue4";
    info.vars.CARLA_UE4_ROOT = p;
  } else {
    const launcher = ["CarlaUE4.sh", "CarlaUnreal.sh", "LinuxNoEditor/CarlaUE4.sh"]
      .find((c) => isFile(path.join(p, c)));
    if (launcher) {
      info.kind = "release";
      info.vars.CARLA_PACKAGE_ROOT = p;
      info.vars.CARLA_TARGET = p;
    } else if (!hasApi) {
      info.detail = "no CARLA here (no launcher, no Unreal project, no PythonAPI)";
      return info;
    }
  }

  if (hasApi) info.vars.CARLA_ROOT = p;

  const bits = [info.kind];
  if (info.flavor) bits.push(info.flavor);
  const br = branchOf(p);
  if (br && br !== "HEAD") bits.push(`branch ${br}`);
  if (!hasApi) bits.push("no PythonAPI/carla — build or install it before the API skills work");
  info.detail = bits.join(", ");
  return info;
}

// Where a CARLA is plausibly found when the user has not said. A guess is only
// ever offered as a candidate to confirm, never selected silently.
const SEARCH = [
  "{cwd}", "{cwd}/carla", "{home}/carla", "{home}/CARLA",
  "{home}/UE58/carla", "{home}/carla-ue58", "{home}/carla-downloads",
  "/opt/carla", "/workspace/carla",
];

function carlaCandidates(cwd) {
  const home = os.homedir();
  const here = cwd || process.cwd();
  const seen = new Map();
  for (const tmpl of SEARCH) {
    const base = tmpl.replace("{cwd}", here).replace("{home}", home);
    if (!isDir(base)) continue;
    let globbed = [];
    try {
      globbed = fs.readdirSync(base)
        .filter((n) => n.startsWith("CARLA_"))
        .sort()
        .map((n) => path.join(base, n));
    } catch (e) { /* unreadable directory is simply not a candidate */ }
    for (const cand of [base, ...globbed]) {
      let key;
      try { key = fs.realpathSync(cand); } catch (e) { continue; }
      if (seen.has(key)) continue;
      const info = detectCarla(cand);
      if (info.kind !== "none") seen.set(key, info);
    }
  }
  return [...seen.values()];
}

module.exports = {
  CONFIG_KEYS, DERIVED_KEYS, PROJECT_CONFIG, VALIDATORS, SEARCH,
  userConfigPath, readConfig, writeConfig, resolve, resolveValid,
  validate, detectCarla, carlaCandidates,
};
