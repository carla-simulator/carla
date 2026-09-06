#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Convert external logs (GPX, MCAP, Traccar JSON) into a run directory, or export a run directory to GPX / MCAP.

Examples:
  python scripts/convert_run.py import --format gpx --input dashcam.gpx --run-id BT-2026-09-05-01 --out-dir _out/runs
  python scripts/convert_run.py import --format mcap --input rosbag.mcap --run-id truck01-0905 --vehicle-class truck
  python scripts/convert_run.py import --format traccar --input positions.json --device-id 42 --run-id fleet-42-0905
  python scripts/convert_run.py export --format mcap --run-dir _out/bhutan_runs/<run> --output run.mcap
"""

import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.adapters import write_run_dir  # noqa: E402
from bhutan_sim.adapters.gpx import read_gpx, write_gpx  # noqa: E402
from bhutan_sim.adapters.traccar import read_traccar  # noqa: E402
from bhutan_sim.quality import assess_run  # noqa: E402
from bhutan_sim.safety_rules import check_samples  # noqa: E402
from bhutan_sim.telemetry import RunManifest, load_run  # noqa: E402


def do_import(args):
    if args.format == "gpx":
        samples = read_gpx(args.input, source=args.source)
    elif args.format == "traccar":
        samples = read_traccar(args.input, device_id=args.device_id, source=args.source)
    else:
        from bhutan_sim.adapters.mcap_io import read_mcap
        samples = read_mcap(args.input, topics=args.topics or None, source=args.source)
    if not samples:
        sys.exit("no position samples found in %s" % args.input)
    run_dir = os.path.join(args.out_dir, args.run_id)
    manifest = RunManifest(run_id=args.run_id, source=args.source, vehicle_class=args.vehicle_class, route_id=args.route_id or ("import:%s" % os.path.basename(args.input)),
                           map_name="real-world" if args.source == "vehicle" else "", odd_zone=args.odd_zone or "", consent_ref=args.consent_ref,
                           notes="Imported from %s (%s) by convert_run.py" % (os.path.basename(args.input), args.format),
                           streams={"video": False, "gnss": True, "imu": False, "can": False, "events": False})
    events = [e.to_dict() for e in check_samples(samples, args.vehicle_class, args.speed_limit_kph)] if args.safety_rules else []
    result = write_run_dir(run_dir, manifest, samples, events)
    rate = len(samples) / max(1e-6, samples[-1]["t"] - samples[0]["t"]) if len(samples) > 1 else 1.0
    quality = assess_run(args.run_id, samples, events, manifest.streams, expected_rate_hz=args.rate_hz or max(0.1, rate),
                         privacy_status="not_required" if args.source == "sim" else "unprocessed")
    with open(os.path.join(run_dir, "quality.json"), "w", encoding="utf-8") as handle:
        json.dump(quality.to_dict(), handle, indent=2, sort_keys=True)
    print("wrote %s: %d samples, %d events, %.2f km, acceptance %.0f%%" % (run_dir, result["samples"], result["events"], quality.distance_km, quality.acceptance_rate * 100))
    print("next: python scripts/upload_run.py --run-dir %s" % run_dir)


def do_export(args):
    run = load_run(args.run_dir)
    manifest = run["manifest"]
    output = args.output or os.path.join(args.run_dir, "%s.%s" % (manifest["run_id"], args.format))
    if args.format == "gpx":
        write_gpx(run["samples"], output, name=manifest["run_id"])
    else:
        from bhutan_sim.adapters.mcap_io import write_mcap
        write_mcap(output, run["samples"], run["events"], metadata={k: manifest.get(k, "") for k in ("run_id", "source", "vehicle_class", "scenario_id", "route_id")})
    print("wrote", output)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    imp = sub.add_parser("import", help="external log -> run directory")
    imp.add_argument("--format", required=True, choices=["gpx", "mcap", "traccar"])
    imp.add_argument("--input", required=True)
    imp.add_argument("--run-id", default="import-%d" % int(time.time()))
    imp.add_argument("--out-dir", default="_out/bhutan_runs")
    imp.add_argument("--source", default="vehicle", choices=["vehicle", "sim"])
    imp.add_argument("--vehicle-class", default="truck", choices=["truck", "shuttle", "car"])
    imp.add_argument("--route-id", default=None)
    imp.add_argument("--odd-zone", default=None)
    imp.add_argument("--consent-ref", default=None)
    imp.add_argument("--device-id", default=None, help="Traccar device id to keep when the export has several")
    imp.add_argument("--topics", nargs="*", default=None, help="MCAP topics to read (default: all JSON position topics)")
    imp.add_argument("--rate-hz", type=float, default=None, help="expected sample rate for the quality gates (default: measured)")
    imp.add_argument("--speed-limit-kph", type=float, default=None)
    imp.add_argument("--no-safety-rules", dest="safety_rules", action="store_false", help="skip SR-01..SR-08 checks on import")
    exp = sub.add_parser("export", help="run directory -> GPX or MCAP")
    exp.add_argument("--format", required=True, choices=["gpx", "mcap"])
    exp.add_argument("--run-dir", required=True)
    exp.add_argument("--output", default=None)
    args = parser.parse_args()
    if args.command == "import":
        do_import(args)
    else:
        do_export(args)


if __name__ == "__main__":
    main()
