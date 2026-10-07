"use strict";
// MCP stdio server. No dependencies on purpose: the surface this needs is four
// JSON-RPC methods, and a CARLA build tool should not drag ~90 transitive
// packages behind it. The response shapes below match what the Python server
// emits, and tests/test_node_parity.py diffs the two.
//
// Two rules for a stdio server:
//   1. stdout carries the JSON-RPC stream — every diagnostic goes to stderr, or
//      the client sees corrupt frames.
//   2. one message per line, newline-delimited.

const skills = require("./skills");

// Newest first. An `initialize` echoes the client's version when we know it, so
// a client on an older revision is not forced to upgrade.
const PROTOCOL_VERSIONS = ["2025-06-18", "2025-03-26", "2024-11-05"];

function version() {
  try {
    return require("../package.json").version;
  } catch (e) {
    return "0.0.0";
  }
}

const NULLABLE_STRING = { anyOf: [{ type: "string" }, { type: "null" }], default: null };

const TOOLS = [
  {
    name: "list_skills",
    description:
`List the available CARLA skills as {name, group, description, available} entries.

The library is the source of procedure for CARLA tasks — match a skill by its
description and call read_skill(name), rather than improvising from the Makefile.

Groups say what a skill binds to: \`python-api\` drives any running server,
\`ue4\`/\`ue5\`/\`ue58\` need that engine's checkout, \`ros2\` covers the native ROS 2
interface. \`available: false\` means something it needs is not configured yet
(see \`unavailable_reason\`) — the skill is still listed, because obtaining that
thing is itself a valid next step. Pass \`group\` to list one group only.`,
    inputSchema: { type: "object", properties: { group: NULLABLE_STRING } },
    handler: (a) => skills.listSkills(a.group === undefined ? null : a.group),
  },
  {
    name: "read_skill",
    description:
`Return a skill's full SKILL.md — the step-by-step procedure and its gotchas.

Call after list_skills once a skill matches the task, and read it before running
the commands it describes. The first line is the skill's absolute directory: the
document's own \`scripts/...\` paths are relative to it, and your working
directory is not. \`name\` is a name from list_skills.`,
    inputSchema: { type: "object", properties: { name: { type: "string" } }, required: ["name"] },
    handler: (a) => skills.readSkill(a.name),
  },
  {
    name: "check_prerequisites",
    description:
`Run a skill's read-only prerequisite checks and return its PASS/WARN/FAIL report.

Call before executing a skill to confirm the environment is ready. Read-only;
does not modify the system. When the check fails, the report ends in a \`needs\`
section naming the paths that are missing or unusable, with candidates found on
this machine — ask the user to choose and record it with set_config.
\`name\` is a name from list_skills.`,
    inputSchema: { type: "object", properties: { name: { type: "string" } }, required: ["name"] },
    handler: (a) => skills.checkPrerequisites(a.name),
  },
  {
    name: "get_config",
    description:
`Report every configurable path, its value, and where that value came from.

Call this before asking the user anything: a key already set in the environment
or the config file needs no prompt. \`candidates\` lists the CARLA installs found
on this machine, each with its flavor and branch, for when CARLA_ROOT is unset —
offer them as choices rather than picking one.`,
    inputSchema: { type: "object", properties: {} },
    handler: () => skills.getConfig(),
  },
  {
    name: "set_config",
    description:
`Persist configured paths so they survive the session. Ask the user first.

\`paths\` maps a key from get_config to an absolute path; an empty value clears
it. Setting CARLA_ROOT also writes the engine-specific variable for whatever that
directory turns out to be, which is what makes the matching ue4/ue5/ue58 skills
available — so ask only for CARLA_ROOT, never for those.

Never guess a path here. Present get_config's \`candidates\` to the user with
their flavor and branch and let them choose, because several checkouts of
different branches on one machine is normal and the wrong one fails slowly.`,
    inputSchema: {
      type: "object",
      properties: { paths: { type: "object", additionalProperties: { type: "string" } } },
      required: ["paths"],
    },
    handler: (a) => skills.setConfig(a.paths),
  },
];

const BY_NAME = new Map(TOOLS.map((t) => [t.name, t]));

// Clients read `content` or `structuredContent`, so both must look exactly like
// the Python server's. Three shapes, and the difference between them is not
// cosmetic — an object goes into structuredContent unwrapped, while a string or
// an array is wrapped in `result` because neither is a valid top-level object.
function toResult(value) {
  if (typeof value === "string") {
    return { content: [{ type: "text", text: value }], structuredContent: { result: value } };
  }
  if (Array.isArray(value)) {
    return {
      content: value.map((v) => ({ type: "text", text: JSON.stringify(v, null, 2) })),
      structuredContent: { result: value },
    };
  }
  return {
    content: [{ type: "text", text: JSON.stringify(value, null, 2) }],
    structuredContent: value,
  };
}

function handleCall(params) {
  const tool = BY_NAME.get(params && params.name);
  if (!tool) throw new Error(`unknown tool '${params && params.name}'`);
  const args = (params && params.arguments) || {};
  for (const req of tool.inputSchema.required || []) {
    if (args[req] === undefined) throw new Error(`missing required argument '${req}'`);
  }
  const value = tool.handler(args);
  return Object.assign(toResult(value), { isError: false });
}

function dispatch(msg) {
  switch (msg.method) {
    case "initialize": {
      const asked = msg.params && msg.params.protocolVersion;
      return {
        protocolVersion: PROTOCOL_VERSIONS.includes(asked) ? asked : PROTOCOL_VERSIONS[0],
        capabilities: { tools: { listChanged: false } },
        serverInfo: { name: "carla-agentic-tools", version: version() },
        instructions: skills.SERVER_INSTRUCTIONS,
      };
    }
    case "ping":
      return {};
    case "tools/list":
      return {
        tools: TOOLS.map((t) => ({
          name: t.name, description: t.description, inputSchema: t.inputSchema,
        })),
      };
    case "tools/call":
      // A tool that throws is a failed call, not a failed request: the client
      // shows the message to the model, which can then correct itself.
      try {
        return handleCall(msg.params);
      } catch (e) {
        return {
          content: [{ type: "text", text: `Error executing tool ${msg.params && msg.params.name}: ${e.message}` }],
          isError: true,
        };
      }
    default:
      return null; // signals method-not-found to the caller
  }
}

function main() {
  let buffer = "";
  const send = (obj) => process.stdout.write(`${JSON.stringify(obj)}\n`);

  process.stdin.setEncoding("utf8");
  process.stdin.on("data", (chunk) => {
    buffer += chunk;
    let nl;
    while ((nl = buffer.indexOf("\n")) !== -1) {
      const line = buffer.slice(0, nl).trim();
      buffer = buffer.slice(nl + 1);
      if (!line) continue;

      let msg;
      try {
        msg = JSON.parse(line);
      } catch (e) {
        send({ jsonrpc: "2.0", id: null, error: { code: -32700, message: "parse error" } });
        continue;
      }
      // A notification carries no id and must never be answered.
      const isNotification = msg.id === undefined || msg.id === null;
      let result;
      try {
        result = dispatch(msg);
      } catch (e) {
        if (!isNotification) {
          send({ jsonrpc: "2.0", id: msg.id, error: { code: -32603, message: e.message } });
        }
        continue;
      }
      if (isNotification) continue;
      if (result === null) {
        send({ jsonrpc: "2.0", id: msg.id, error: { code: -32601, message: `method not found: ${msg.method}` } });
      } else {
        send({ jsonrpc: "2.0", id: msg.id, result });
      }
    }
  });

  // The client closing stdin is how a stdio server is told to stop.
  process.stdin.on("end", () => process.exit(0));
}

module.exports = { main, dispatch, TOOLS, PROTOCOL_VERSIONS, version };
