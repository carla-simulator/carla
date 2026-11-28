"""T2 — the CI smoke suite, run exactly as CI runs it (Util/Tools/RunSmokeTests.sh)."""

import xml.etree.ElementTree as ET

from harness.core import test, check, REPO, PYTHON


@test("T2.smoke")
def smoke(ctx):
    rc, out = ctx.sh(["bash", REPO / "Util" / "Tools" / "RunSmokeTests.sh",
                      "--connect-timeout", "300", "--results-dir", ctx.out, PYTHON],
                     ok_rc=None, timeout=3 * 3600)
    xmls = sorted(ctx.out.glob("smoke-*.xml"))
    check(xmls, f"no JUnit XML produced (rc={rc}):\n{out[-2000:]}")
    failed, counts = [], {"PASS": 0, "FAIL": 0, "SKIP": 0}
    for case in ET.parse(xmls[0]).getroot().iter("testcase"):
        name = f"{case.get('classname', '').split('.')[-1]}.{case.get('name')}"
        err = case.find("failure") if case.find("failure") is not None else case.find("error")
        if err is not None:
            status, msg = "FAIL", (err.get("message") or "") + "\n" + (err.text or "")[-1500:]
            failed.append(name)
        elif case.find("skipped") is not None:
            status, msg = "SKIP", case.find("skipped").get("message", "")
        else:
            status, msg = "PASS", ""
        counts[status] += 1
        ctx.subresults.append({"id": f"T2.smoke.{name}", "tier": "T2", "status": status,
                               "msg": msg.strip(), "secs": float(case.get("time", 0)),
                               "metrics": {}, "notes": [], "log": str(ctx.log_path)})
    check(counts["PASS"] + counts["FAIL"] > 0, "smoke XML has no test cases")
    check(not failed, f"{len(failed)} smoke tests failed: {', '.join(failed)}")
    return counts
