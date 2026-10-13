"use strict";
// Skill discovery and the four things the tools do with a skill. The Node half
// of src/carla_agentic_tools/server.py; tests/test_node_parity.py compares the
// two servers' answers.

const fs = require("fs");
const path = require("path");
const { spawnSync } = require("child_process");
const cfg = require("./config");

// Which env var makes a group's skills usable. A group with no entry here is
// always available. `setup` is deliberately absent: gating the bootstrap skills
// on an existing CARLA would hide exactly the ones a user with nothing needs.
const GROUP_REQUIREMENTS = {
  ue4: [["CARLA_UE4_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT"],
    "a CARLA ue4-dev checkout or an extracted release"],
  ue5: [["CARLA_UE5_ROOT"], "a CARLA ue5-dev checkout (UE 5.5)"],
  ue58: [["CARLA_UE58_ROOT"], "a CARLA ue58-dev checkout (UE 5.8)"],
  "scenario-runner": [["SCENARIO_RUNNER_ROOT"], "a scenario_runner checkout"],
  leaderboard: [["LEADERBOARD_ROOT"], "a leaderboard checkout (plus its matching scenario_runner)"],
  scenic: [["SCENIC_ROOT"], "a Scenic install"],
};

const ENGINE_GROUPS = ["ue4", "ue5", "ue58"];

// What each installer skill creates. An unset key can be offered as "get it for
// me", and a skill is never gated on something it exists to produce.
const PROVIDES = {
  "download-carla": ["CARLA_ROOT"],
  "install-python-api": ["PYTHON"],
  "install-scenario-runner": ["SCENARIO_RUNNER_ROOT"],
  "install-leaderboard": ["LEADERBOARD_ROOT", "SCENARIO_RUNNER_ROOT"],
  "install-scenic": ["SCENIC_ROOT", "PYTHON"],
};

const PROVIDERS = {};
for (const [name, keys] of Object.entries(PROVIDES)) {
  for (const key of [...keys].reverse()) PROVIDERS[key] = name;
}
for (const [name, keys] of Object.entries(PROVIDES)) PROVIDERS[keys[0]] = name;

const SERVER_INSTRUCTIONS = `CARLA build, packaging, cooking, map, vehicle, asset, and server/simulation
tasks have vetted procedures in this server's skill library, each with its
failure modes encoded. The skills are the source of truth for these tasks;
the raw Makefile and Util/BuildTools scripts are a fallback when none matches.
list_skills finds a skill, read_skill(name) returns its procedure,
check_prerequisites(name) verifies its environment.

Paths are configured on first need, not up front. When check_prerequisites
reports a "needs" section, ask the user which path to use — offering each
candidate with its flavor and branch, the install_skill that would obtain it,
and typing a path — then record the answer with set_config so it survives the
session. Never guess a path: several CARLA checkouts on one machine is normal
and the wrong one fails slowly. CARLA_ROOT is the only CARLA path to ask for;
set_config derives the engine-specific variable that gates ue4/ue5/ue58.
`;

// The library is Skills/ at the carla repo root, three levels above lib/.
// CARLA_SKILLS_DIR overrides it, for authoring against another working tree.
function resolveSkillsDir() {
  if (process.env.CARLA_SKILLS_DIR) return path.resolve(process.env.CARLA_SKILLS_DIR);
  return path.resolve(path.join(__dirname, "..", "..", "..", "Skills"));
}

const SKILLS_DIR = resolveSkillsDir();

function skillDirs() {
  const out = [];
  const groups = safeReaddir(SKILLS_DIR);
  for (const g of groups) {
    const gp = path.join(SKILLS_DIR, g);
    if (!isDir(gp)) continue;
    // Grouped: skills/<group>/<name>/SKILL.md
    for (const n of safeReaddir(gp)) {
      if (isFile(path.join(gp, n, "SKILL.md"))) out.push(path.join(gp, n));
    }
    // Flat: skills/<name>/SKILL.md, still accepted for a drop-in directory.
    if (isFile(path.join(gp, "SKILL.md"))) out.push(gp);
  }
  return out.sort((a, b) => {
    const ka = [path.basename(path.dirname(a)), path.basename(a)].join("/");
    const kb = [path.basename(path.dirname(b)), path.basename(b)].join("/");
    return ka < kb ? -1 : ka > kb ? 1 : 0;
  });
}

const isDir = (p) => { try { return fs.statSync(p).isDirectory(); } catch (e) { return false; } };
const isFile = (p) => { try { return fs.statSync(p).isFile(); } catch (e) { return false; } };
const safeReaddir = (p) => { try { return fs.readdirSync(p).sort(); } catch (e) { return []; } };

function groupOf(d) {
  const parent = path.dirname(d);
  return path.resolve(parent) === SKILLS_DIR ? "" : path.basename(parent);
}

function findSkill(name) {
  return skillDirs().find((d) => path.basename(d) === name) || null;
}

function descriptionOf(d) {
  for (const line of fs.readFileSync(path.join(d, "SKILL.md"), "utf8").split("\n")) {
    if (line.startsWith("description:")) return line.slice("description:".length).trim();
  }
  return "";
}

// Each env.sh documents the variables it reads in a header block. Parsing that
// is what lets check_prerequisites report exactly which keys a skill needs.
// A key whose doc names a default is optional.
const VAR_DOC = /^#\s{2,}([A-Z][A-Z0-9_]{2,})\s{2,}(\S.*)$/;

function declaredVars(skill) {
  const envSh = path.join(skill, "scripts", "env.sh");
  const out = {};
  if (!isFile(envSh)) return out;
  const lines = fs.readFileSync(envSh, "utf8").split("\n").slice(0, 40);
  for (const line of lines) {
    const m = VAR_DOC.exec(line);
    if (m) {
      const doc = m[2].trim();
      const lower = doc.toLowerCase();
      out[m[1]] = { doc, required: !(lower.includes("(default") || lower.includes("default:")) };
    }
  }
  return out;
}

function groupAvailable(group) {
  const req = GROUP_REQUIREMENTS[group];
  if (!req) return { ok: true, why: "" };
  const [vars_, what] = req;
  const broken = [];
  for (const v of vars_) {
    const { value, problem } = cfg.resolveValid(v);
    if (value && !problem) return { ok: true, why: "" };
    if (value) broken.push(`${v}: ${problem}`);
  }
  if (ENGINE_GROUPS.includes(group)) {
    const { value: root, problem } = cfg.resolveValid("CARLA_ROOT");
    if (root && !problem) {
      const flavor = cfg.detectCarla(root).flavor;
      if (flavor === group) return { ok: true, why: "" };
      broken.push(`CARLA_ROOT is a ${flavor || "unrecognised"} CARLA, not ${group}`);
    } else if (root) {
      broken.push(`CARLA_ROOT: ${problem}`);
    }
  }
  if (broken.length) {
    // A configured-but-unusable path is a different problem from an unset one,
    // and saying which stops the user re-entering the same wrong answer.
    return { ok: false, why: `configured but unusable — ${broken.join("; ")} (needs ${what})` };
  }
  return {
    ok: false,
    why: ENGINE_GROUPS.includes(group)
      ? `not configured: set CARLA_ROOT to your CARLA, or ${vars_.join("/")} directly (needs ${what})`
      : `not configured: ${vars_.join("/")} is unset (needs ${what})`,
  };
}

// Group requirement, plus every required key this one skill declares.
// navigate-to sits in the ungated python-api group but imports `agents`, which
// ships only inside a CARLA tree — so it needs CARLA_ROOT while its neighbours
// need only the carla wheel.
function skillAvailable(skill, group) {
  const g = groupAvailable(group);
  if (!g.ok) return g;
  const name = path.basename(skill);
  for (const [key, { doc, required }] of Object.entries(declaredVars(skill))) {
    if (!required || !(key in cfg.CONFIG_KEYS)) continue;
    if ((PROVIDES[name] || []).includes(key)) continue;
    const { value, problem } = cfg.resolveValid(key);
    if (!value) return { ok: false, why: `not configured: ${key} is unset (${doc})` };
    if (problem) return { ok: false, why: `configured but unusable — ${key}: ${problem}` };
  }
  return { ok: true, why: "" };
}

function unmetKeys(skill) {
  const lines = [];
  for (const [key, { doc }] of Object.entries(declaredVars(skill))) {
    if (!(key in cfg.CONFIG_KEYS)) continue;
    const { value, problem } = cfg.resolveValid(key);
    if (value && !problem) continue;
    lines.push(`key: ${key}`);
    lines.push(`  what: ${doc}`);
    if (problem && value) lines.push(`  current value is unusable: ${problem}`);
    if (PROVIDERS[key]) lines.push(`  install_skill: ${PROVIDERS[key]}`);
    if (key === "CARLA_ROOT") {
      for (const c of cfg.carlaCandidates()) lines.push(`  candidate: ${c.path}  (${c.detail})`);
    }
  }
  if (!lines.length) return "";
  return "Ask the user to choose, then call set_config. Offer each candidate with\n"
    + "its flavor and branch, the install_skill, and typing a path.\n"
    + lines.join("\n");
}

// --- the tools -------------------------------------------------------------

function listSkills(group) {
  const out = [];
  for (const d of skillDirs()) {
    const g = groupOf(d);
    if (group !== undefined && group !== null && g !== group) continue;
    const { ok, why } = skillAvailable(d, g);
    const entry = { name: path.basename(d), group: g, description: descriptionOf(d), available: ok };
    if (why) entry.unavailable_reason = why;
    out.push(entry);
  }
  return out;
}

function readSkill(name) {
  const d = findSkill(name);
  if (!d) throw new Error(`unknown skill '${name}'; see list_skills()`);
  // The document reaches its scripts and references by paths relative to its own
  // directory, and the client's working directory is the user's project. An MCP
  // client has never seen a filesystem path for this skill, so state it here or
  // every `bash scripts/check_env.sh` in the body is unrunnable.
  // The announce line covers the case where the body's prerequisite step is
  // skipped: scripts/check_env.sh prints the same banner itself, but a skill
  // that needs no preflight would otherwise do its work with nothing on screen
  // naming it.
  const header = `Skill directory: ${d}\n`
    + "Every `scripts/...` and `references/...` path below is relative to that "
    + "directory. Prefix them with it before running anything.\n"
    + `Announce yourself first: run \`printf '\\n\\033[1;36m>> using skill: `
    + `${name}\\033[0m\\n'\` before the first step, so a terminal recording `
    + "shows which skill is acting.\n\n";
  return header + fs.readFileSync(path.join(d, "SKILL.md"), "utf8");
}

function checkPrerequisites(name) {
  const d = findSkill(name);
  if (!d) throw new Error(`unknown skill '${name}'; see list_skills()`);
  const script = path.join(d, "scripts", "check_env.sh");
  if (!isFile(script)) {
    throw new Error(`skill '${name}' has no prerequisite checks (scripts/check_env.sh)`);
  }
  const proc = spawnSync("bash", [script], {
    encoding: "utf8", timeout: 120000, maxBuffer: 16 * 1024 * 1024,
  });
  const code = proc.status === null ? 1 : proc.status;
  const report = `exit=${code}\n--- stdout ---\n${proc.stdout || ""}\n`
    + `--- stderr ---\n${proc.stderr || ""}`;
  // Only when the preflight actually failed. Several keys have a working default
  // (PYTHON falls back to python3), so asking about one the check just passed
  // with would be a prompt for nothing.
  if (code === 0) return report;
  const needs = unmetKeys(d);
  return report + (needs ? `\n--- needs ---\n${needs}` : "");
}

function getConfig() {
  const entries = {};
  const all = { ...cfg.CONFIG_KEYS };
  for (const k of cfg.DERIVED_KEYS) all[k] = "derived from CARLA_ROOT";
  for (const [key, what] of Object.entries(all)) {
    const { value, source, problem } = cfg.resolveValid(key);
    entries[key] = { value, source, what, usable: Boolean(value) && !problem };
    if (problem && value) entries[key].problem = problem;
  }
  const out = { config_file: cfg.userConfigPath(), keys: entries };
  if (!entries.CARLA_ROOT.value) out.candidates = cfg.carlaCandidates();
  return out;
}

function setConfig(paths) {
  const updates = {};
  const notes = [];
  for (const [key, raw] of Object.entries(paths || {})) {
    if (!(key in cfg.CONFIG_KEYS) && !cfg.DERIVED_KEYS.includes(key)) {
      throw new Error(`unknown key '${key}'; see get_config()`);
    }
    if (!raw) {
      updates[key] = "";
      notes.push(`${key} cleared`);
      continue;
    }
    const p = path.resolve(raw.replace(/^~(?=$|\/)/, require("os").homedir()));
    // Reject here rather than storing it: a path that fails validation would
    // otherwise leave the group unavailable with no sign the answer was wrong.
    const { ok, why } = cfg.validate(key, p);
    if (!ok) throw new Error(`${key}: ${why}`);
    updates[key] = p;
    if (key === "CARLA_ROOT") {
      const info = cfg.detectCarla(p);
      if (info.kind === "none") throw new Error(`CARLA_ROOT: ${info.detail} at ${p}`);
      Object.assign(updates, info.vars);
      const also = Object.keys(info.vars).filter((k) => k !== "CARLA_ROOT");
      notes.push(`CARLA_ROOT is ${info.detail}; also set ${also.join(", ")}`);
    } else {
      notes.push(`${key} = ${p}`);
    }
  }
  const written = cfg.writeConfig(updates);
  const shadowed = Object.keys(updates).filter((k) => process.env[k] && process.env[k] !== updates[k]);
  if (shadowed.length) {
    notes.push("NOTE: an exported environment variable still overrides "
      + `${shadowed.join(", ")} — unset it or the config is ignored`);
  }
  return `wrote ${written}\n` + notes.map((n) => `  ${n}`).join("\n");
}

module.exports = {
  SKILLS_DIR, SERVER_INSTRUCTIONS, GROUP_REQUIREMENTS, PROVIDES, PROVIDERS,
  skillDirs, findSkill, groupOf, declaredVars, groupAvailable, skillAvailable, unmetKeys,
  listSkills, readSkill, checkPrerequisites, getConfig, setConfig,
};
