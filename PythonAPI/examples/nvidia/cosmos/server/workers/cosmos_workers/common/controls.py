"""Hand server-produced control videos back to the client next to the generated video."""

from __future__ import annotations

import re
import shutil
from pathlib import Path
from typing import Any


def canonical(name: str) -> str:
    """``camera:front:wide:120fov`` -> ``camera_front_wide_120fov`` (file-name safe view id)."""
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", name)


def rendered_control_files(inputs: dict[str, Any], views: list[str], out_dir: Path) -> list[dict[str, Any]]:
    """Copy every control the *server* produced (rendered from a scene package: the API server sets
    ``rendered`` on the spec after the renderer ran) into ``out_dir`` and return their file entries.

    Uploaded controls are not returned — the client already has them.  Names follow the Transfer 2.5
    worker's convention: ``control_<hint>.mp4`` for a single view, ``control_<hint>_<view>.mp4`` otherwise,
    ``kind`` = ``control`` so the viewer shows them as a panel next to the input and the result.
    """
    files: list[dict[str, Any]] = []
    for hint, spec in (inputs.get("controls") or {}).items():
        if not spec.get("rendered"):
            continue
        paths = spec.get("paths") or ({views[0]: spec["path"]} if spec.get("path") and len(views) == 1 else {})
        for view in views:
            src = paths.get(view)
            if not src or not Path(src).is_file():
                continue
            name = f"control_{hint}.mp4" if len(views) == 1 else f"control_{hint}_{canonical(view)}.mp4"
            shutil.copyfile(src, out_dir / name)
            files.append({"name": name, "view": view, "kind": "control"})
    return files
