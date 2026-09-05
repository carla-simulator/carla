# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generate the versioned scenario library from the taxonomy."""

from __future__ import annotations

import json
import os
from typing import Any, Dict, Iterable, List, Optional

from .scenario import ScenarioTemplate
from .taxonomy import Taxonomy

DEFAULT_LIBRARY_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "scenarios", "library.json")


def generate_library(taxonomy: Optional[Taxonomy] = None, base_seed: int = 0,
                     vehicle_class: str = "truck", min_total: int = 100) -> List[ScenarioTemplate]:
    """Expand every family into its declared number of variants.

    Raises ``ValueError`` if the taxonomy cannot produce ``min_total``
    templates, which protects the Month-3 "100+ scenarios" KPI from silently
    regressing when families are edited.
    """
    taxonomy = taxonomy or Taxonomy.load()
    templates: List[ScenarioTemplate] = []
    for family in taxonomy.families:
        for index in range(family.variants):
            templates.append(family.variant(index, base_seed=base_seed, vehicle_class=vehicle_class))
    ids = [t.id for t in templates]
    if len(ids) != len(set(ids)):
        raise ValueError("scenario ids are not unique")
    if len(templates) < min_total:
        raise ValueError("taxonomy produced %d templates, fewer than the required %d" % (len(templates), min_total))
    return templates


def library_manifest(templates: Iterable[ScenarioTemplate], taxonomy_version: str) -> Dict[str, Any]:
    items = [t.to_dict() for t in templates]
    families = sorted({t["family"] for t in items})
    return {
        "schema_version": "1.0",
        "taxonomy_version": taxonomy_version,
        "count": len(items),
        "families": families,
        "scenarios": items,
    }


def write_library(path: Optional[str] = None, taxonomy: Optional[Taxonomy] = None, base_seed: int = 0) -> Dict[str, Any]:
    taxonomy = taxonomy or Taxonomy.load()
    templates = generate_library(taxonomy, base_seed=base_seed)
    manifest = library_manifest(templates, taxonomy.version)
    path = path or DEFAULT_LIBRARY_PATH
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(manifest, handle, indent=2, sort_keys=True)
        handle.write("\n")
    return manifest


def load_library(path: Optional[str] = None) -> List[ScenarioTemplate]:
    path = path or DEFAULT_LIBRARY_PATH
    with open(path, "r", encoding="utf-8") as handle:
        manifest = json.load(handle)
    return [ScenarioTemplate.from_dict(item) for item in manifest["scenarios"]]
