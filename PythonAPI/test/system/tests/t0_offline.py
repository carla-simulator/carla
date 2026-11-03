"""T0 — checks that need no server."""

import json
import py_compile
import re
import select
import subprocess
import tempfile
import time

from harness.core import test, check, Fail, Skip, REPO, SKILLS, EXAMPLES, PYTHON

EXPECTED_SKILLS = 63


def _skill_dirs():
    return sorted(p.parent for p in SKILLS.glob("*/*/SKILL.md"))


def _frontmatter(path):
    text = path.read_text()
    m = re.match(r"^---\n(.*?)\n---\n", text, re.S)
    if not m:
        return None
    keys = {}
    for line in m.group(1).splitlines():
        km = re.match(r"^([A-Za-z_]+):\s*(.*)$", line)
        if km:
            keys[km.group(1)] = km.group(2).strip().strip("'\"")
    return keys


@test("T0.unit")
def unit(ctx):
    # Own config: the directory's unittest.cfg would drop a JUnit XML into the source tree.
    cfg = ctx.out / "unittest.cfg"
    cfg.write_text("[unittest]\n")
    rc, out = ctx.sh([PYTHON, "-m", "nose2", "-v", "-c", cfg],
                     cwd=REPO / "PythonAPI" / "test" / "unit", ok_rc=None, timeout=600)
    ran = re.search(r"Ran (\d+) tests?", out)
    check(ran and int(ran.group(1)) > 0, f"no unit tests ran:\n{out[-2000:]}")
    failed = re.findall(r"^(?:FAIL|ERROR): (\S+ \([^)]+\))", out, re.M)
    check(rc == 0, f"{len(failed)} unit tests failed: {', '.join(failed)}\n" +
          out[out.find("======"):][-2000:])
    return {"tests": int(ran.group(1))}


@test("T0.skills-frontmatter")
def skills_frontmatter(ctx):
    dirs = _skill_dirs()
    problems = []
    for d in dirs:
        fm = _frontmatter(d / "SKILL.md")
        if fm is None:
            problems.append(f"{d.name}: no frontmatter")
            continue
        missing = [k for k in ("name", "description", "compatibility", "metadata") if k not in fm]
        if missing:
            problems.append(f"{d.name}: missing {missing}")
        if fm.get("name") != d.name:
            problems.append(f"{d.name}: name is {fm.get('name')!r}")
    check(len(dirs) == EXPECTED_SKILLS, f"found {len(dirs)} skills, AGENTS.md lists {EXPECTED_SKILLS}")
    check(not problems, "\n".join(problems))
    return {"skills": len(dirs)}


@test("T0.skills-compile")
def skills_compile(ctx):
    bad = []
    py = sorted(SKILLS.glob("**/scripts/**/*.py"))
    sh = sorted(SKILLS.glob("**/scripts/**/*.sh")) + [SKILLS / "_common" / "env_common.sh"]
    with tempfile.TemporaryDirectory() as tmp:
        for f in py:
            try:
                py_compile.compile(str(f), cfile=f"{tmp}/x.pyc", doraise=True)
            except py_compile.PyCompileError as e:
                bad.append(f"{f.relative_to(REPO)}: {e.msg.strip()}")
    for f in sh:
        r = subprocess.run(["bash", "-n", str(f)], capture_output=True, text=True)
        if r.returncode:
            bad.append(f"{f.relative_to(REPO)}: {r.stderr.strip()}")
    check(not bad, "\n".join(bad))
    return {"python_files": len(py), "shell_files": len(sh)}


@test("T0.skills-help")
def skills_help(ctx):
    bad = []
    scripts = sorted(SKILLS.glob("python-api/*/scripts/*.py"))
    for f in scripts:
        rc, out = ctx.sh([PYTHON, f, "--help"], ok_rc=None, timeout=60)
        if rc != 0 or "usage" not in out.lower():
            bad.append(f"{f.relative_to(REPO)} rc={rc}: {out.strip()[-300:]}")
    check(not bad, "\n".join(bad))
    return {"scripts": len(scripts)}


@test("T0.examples-compile")
def examples_compile(ctx):
    bad = []
    files = sorted(EXAMPLES.glob("**/*.py"))
    with tempfile.TemporaryDirectory() as tmp:
        for f in files:
            try:
                py_compile.compile(str(f), cfile=f"{tmp}/x.pyc", doraise=True)
            except py_compile.PyCompileError as e:
                bad.append(f"{f.relative_to(REPO)}: {e.msg.strip()}")
    check(not bad, "\n".join(bad))
    return {"files": len(files)}


@test("T0.doc-gen")
def doc_gen(ctx):
    target = "Docs/python_api.md"
    dirty = subprocess.run(["git", "status", "--porcelain", target], cwd=REPO,
                           capture_output=True, text=True).stdout.strip()
    if dirty:
        raise Skip(f"{target} has local changes; not overwriting it")
    try:
        ctx.sh([PYTHON, "doc_gen.py"], cwd=REPO / "PythonAPI" / "docs", timeout=300)
        diff = subprocess.run(["git", "diff", "--stat", "--", target], cwd=REPO,
                              capture_output=True, text=True).stdout.strip()
        if diff:
            full = subprocess.run(["git", "diff", "--", target], cwd=REPO,
                                  capture_output=True, text=True).stdout
            (ctx.out / "python_api.diff").write_text(full)
        check(not diff, f"{target} is stale relative to PythonAPI/docs/*.yml: {diff} "
                        f"(diff saved as python_api.diff)")
    finally:
        subprocess.run(["git", "checkout", "--", target], cwd=REPO)


@test("T0.agentic-tools-node")
def agentic_node(ctx):
    rc, out = ctx.sh(["node", "test/node_smoke.js"], cwd=REPO / "Util" / "AgenticTools",
                     ok_rc=None, timeout=300)
    check(rc == 0, f"node_smoke.js rc={rc}:\n{out[-2000:]}")


@test("T0.agentic-tools-pytest")
def agentic_pytest(ctx):
    if subprocess.run([PYTHON, "-c", "import pytest"], capture_output=True).returncode:
        raise Skip("pytest not installed")
    rc, out = ctx.sh([PYTHON, "-m", "pytest", "-q", "tests"], cwd=REPO / "Util" / "AgenticTools",
                     ok_rc=None, timeout=900)
    m = re.search(r"(\d+) passed", out)
    check(rc == 0, f"pytest rc={rc}:\n{out[-2500:]}")
    return {"passed": int(m.group(1)) if m else None}


def _rpc(proc, msg, want_id, timeout=30):
    proc.stdin.write((json.dumps(msg) + "\n").encode())
    proc.stdin.flush()
    if want_id is None:
        return None
    end = time.time() + timeout
    while time.time() < end:
        r, _, _ = select.select([proc.stdout], [], [], 1)
        if r:
            line = proc.stdout.readline()
            if not line:
                break
            try:
                obj = json.loads(line)
            except ValueError:
                continue
            if obj.get("id") == want_id:
                return obj
    raise Fail(f"no JSON-RPC reply to id {want_id}")


@test("T0.mcp-list-skills")
def mcp_list_skills(ctx):
    proc = subprocess.Popen(["node", "Util/AgenticTools/bin/carla-agentic-tools.js"], cwd=REPO,
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL)
    try:
        _rpc(proc, {"jsonrpc": "2.0", "id": 1, "method": "initialize",
                    "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                               "clientInfo": {"name": "system-tests", "version": "1"}}}, 1)
        _rpc(proc, {"jsonrpc": "2.0", "method": "notifications/initialized"}, None)
        tools = _rpc(proc, {"jsonrpc": "2.0", "id": 2, "method": "tools/list"}, 2)
        names = {t["name"] for t in tools["result"]["tools"]}
        want = {"list_skills", "read_skill", "check_prerequisites", "get_config", "set_config"}
        check(want <= names, f"MCP tools missing: {want - names}")
        r = _rpc(proc, {"jsonrpc": "2.0", "id": 3, "method": "tools/call",
                        "params": {"name": "list_skills", "arguments": {}}}, 3)
        text = "".join(c.get("text", "") for c in r["result"]["content"])
        (ctx.out / "list_skills.txt").write_text(text)
        missing = [d.name for d in _skill_dirs() if d.name not in text]
        check(not missing, f"list_skills does not mention: {missing}")
        r = _rpc(proc, {"jsonrpc": "2.0", "id": 4, "method": "tools/call",
                        "params": {"name": "read_skill", "arguments": {"name": "spawn-vehicles"}}}, 4)
        body = "".join(c.get("text", "") for c in r["result"]["content"])
        check("vehicles.py" in body, "read_skill(spawn-vehicles) did not return the procedure")
        return {"tools": len(names)}
    finally:
        proc.kill()
