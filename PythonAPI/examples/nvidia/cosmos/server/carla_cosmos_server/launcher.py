"""``carla-cosmos-server`` / ``python -m carla_cosmos_server``: the container entry point.

1. read settings, create ``/state``;
2. load/bootstrap tokens (prints the initial token once);
3. pick the runtime profile (auto from ``nvidia-smi`` + ``--mode`` or ``--profile``);
4. start one worker process per profile entry in its venv;
5. serve the API with uvicorn; readiness flips once every worker loaded and
   passed its smoke sample.
"""

from __future__ import annotations

import argparse
import logging
import os
import sys
from pathlib import Path

import uvicorn

from . import __version__
from .app import create_app
from .auth import TokenStore, bootstrap
from .config import Settings, default_run_dir
from .profiles import MODES, Profile, detect_gpus, load_profiles, select_profile
from .workers_rpc import WorkerHandle, spawn_worker

log = logging.getLogger("carla_cosmos_server")


def build_workers(settings: Settings, profile) -> list[WorkerHandle]:
    handles: list[WorkerHandle] = []
    for spec in profile.workers:
        wt = spec.worker_type
        if not wt.implemented:
            log.error("worker '%s' (type %s) is not implemented in this build; it will report state 'error'",
                      spec.name, spec.type)
            h = WorkerHandle(name=spec.name, backends=spec.backends, socket=settings.run_dir / f"{spec.name}.sock",
                             type=spec.type, gpus=spec.gpus, parallel=spec.parallel)
            h.state, h.error, h.smoke_ok = "error", f"worker type '{spec.type}' not implemented yet", False
            handles.append(h)
            continue
        env = {
            "COSMOS_MODELS_DIR": str(settings.models_dir),
            "COSMOS_GUARDRAILS": "1" if settings.guardrails else "0",
            "COSMOS_LOG_LEVEL": settings.log_level,
        }
        handles.append(spawn_worker(
            name=spec.name, type_=spec.type, backends=spec.backends, gpus=spec.gpus,
            python=settings.venv_python(spec.venv or wt.venv), module=wt.module,
            socket=settings.run_dir / f"{spec.name}.sock", extra_args=spec.args, env=env,
            log_dir=settings.state_dir / "logs", parallel=spec.parallel))
    return handles


def describe_layout(profile: Profile) -> list[str]:
    """One line per worker: name, type, GPUs and how a query is spread over them."""
    lines = []
    for w in profile.workers:
        par = " ".join(f"{k}={v}" for k, v in w.parallel.items())
        lines.append(f"  {w.name:<16} {w.type:<14} gpus={w.gpus}{'  ' + par if par else ''}  {' '.join(w.args)}")
    shared = profile.shared_gpus()
    if shared:
        lines.append(f"  shared GPUs (one query at a time): " +
                     ", ".join(f"{g}: {'+'.join(names)}" for g, names in sorted(shared.items())))
    return lines


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="carla-cosmos-server", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--state", help="state directory (COSMOS_STATE, default /state)")
    p.add_argument("--host", help="bind address (COSMOS_HOST, default 0.0.0.0)")
    p.add_argument("--port", type=int, help="port (COSMOS_PORT, default 8000)")
    p.add_argument("--profile", help="runtime profile name or 'auto' (COSMOS_PROFILE)")
    p.add_argument("--mode", choices=MODES,
                   help="planned layout: latency (one query uses every GPU) | throughput (one worker per model on "
                        "disjoint GPUs) | auto = latency on <= 4 GPUs (COSMOS_MODE); YAML profiles ignore it")
    p.add_argument("--profiles-dir", help="directory with profile YAMLs")
    p.add_argument("--list-profiles", action="store_true", help="print profiles and detected GPUs, then exit")
    p.add_argument("--version", action="version", version=__version__)
    args = p.parse_args(argv)

    settings = Settings()
    if args.state:
        settings.state_dir = Path(args.state)
        if not os.environ.get("COSMOS_RUN_DIR"):
            settings.run_dir = default_run_dir(settings.state_dir)
    if args.host:
        settings.host = args.host
    if args.port:
        settings.port = args.port
    if args.profile:
        settings.profile = args.profile
    if args.mode:
        settings.mode = args.mode
    if settings.mode not in MODES:
        p.error(f"--mode/COSMOS_MODE must be one of {MODES}, got '{settings.mode}'")
    if args.profiles_dir:
        settings.profiles_dir = Path(args.profiles_dir)

    logging.basicConfig(level=settings.log_level.upper(),
                        format="%(asctime)s %(levelname)s %(name)s: %(message)s", stream=sys.stderr)

    profiles = load_profiles(settings.profiles_dir)
    gpus = detect_gpus()
    if args.list_profiles:
        print(f"GPUs: {[(g.index, g.name, g.memory_mib) for g in gpus] or 'none'}   image weights: {settings.image_variant}"
              f"   mode: {settings.mode}")
        for prof in sorted(profiles.values(), key=lambda x: x.priority):
            tag = "auto-match" if prof.accepts(gpus) else ("manual" if prof.match is None else "no match")
            print(f"  {prof.name:<12} [{tag}] {prof.description.strip()}")
        try:
            planned = select_profile(profiles, gpus, "auto", settings.image_variant, settings.mode)
            print(f"auto -> {planned.name} (mode {planned.mode}): {planned.description}")
            print("\n".join(describe_layout(planned)))
        except ValueError as exc:
            print(f"auto -> none ({exc})")
        return 0

    settings.ensure_dirs()
    tokens = TokenStore(settings.tokens_file)
    bootstrap(tokens, settings.initial_token_file, settings.bootstrap_token)

    try:
        profile = select_profile(profiles, gpus, settings.profile, settings.image_variant, settings.mode)
    except ValueError as exc:
        log.error("%s", exc)
        return 2
    errors = profile.validate()
    if errors:
        log.error("profile '%s' is invalid: %s", profile.name, "; ".join(errors))
        return 2
    log.info("carla-cosmos-server %s — profile '%s' (mode %s: %s), %d GPU(s) detected, image weights '%s', state %s, "
             "sockets in %s\n%s", __version__, profile.name, profile.mode, profile.description.strip(), len(gpus),
             settings.image_variant, settings.state_dir, settings.run_dir, "\n".join(describe_layout(profile)))

    workers = build_workers(settings, profile)
    app = create_app(settings, tokens, workers, profile_name=profile.name, mode=profile.mode)
    uvicorn.run(app, host=settings.host, port=settings.port, log_level=settings.log_level.lower(),
                access_log=False, timeout_keep_alive=60)
    return 0


if __name__ == "__main__":
    sys.exit(main())
