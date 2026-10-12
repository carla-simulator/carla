#!/usr/bin/env node
// Entry point for the repo's .mcp.json and the plugin manifests.
// Plain Node, no dependencies; skills that need the CARLA client use `PYTHON`.
"use strict";

const [major] = process.versions.node.split(".").map(Number);
if (major < 12) {
  process.stderr.write(
    `carla-agentic-tools needs Node 12 or newer (running ${process.versions.node}).\n`);
  process.exit(1);
}

require("../lib/server").main();
