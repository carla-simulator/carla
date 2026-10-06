#!/usr/bin/env node
// Entry point for `npx -y github:carla-simulator/carla-agentic-tools`,
// and for the Claude Code plugin, which runs this file from its own checkout.
//
// Self-contained: the skills ship alongside this file and the server is plain Node,
// so nothing here needs Python, uv, or pipx. The skills themselves still shell
// out to bash and, for the CARLA client, to whatever interpreter `PYTHON` names
// — that is the user's environment, not this server's runtime.
"use strict";

const [major] = process.versions.node.split(".").map(Number);
if (major < 12) {
  process.stderr.write(
    `carla-agentic-tools needs Node 12 or newer (running ${process.versions.node}).\n`);
  process.exit(1);
}

require("../lib/server").main();
