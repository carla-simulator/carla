---
name: carla-vehicle-authoring
description: Create, migrate, improve, or debug detailed vehicle assets for CARLA using Blender and Unreal, including reference-based geometry, materials, wheels, physics, lights, and runtime validation. Use for vehicle asset authoring rather than ordinary traffic spawning or map decoration.
license: MIT
metadata:
  group: ue58
  compatibility: Blender and a built CARLA UE5.8 checkout with its matching content and engine. Runtime validation needs the matching CARLA Python API and a dedicated server.
---

# CARLA vehicle authoring

Produce an editable, reproducible vehicle that looks convincing in CARLA cameras and behaves correctly through the CARLA API. Scale work to the request: a tire correction does not require rebuilding the entire vehicle.

## Establish the target

Source [scripts/env.sh](scripts/env.sh) using its absolute path to load the library's recorded configuration while preserving explicit environment overrides. Use `CARLA_UE58_ROOT` and `CARLA_UNREAL_ENGINE_PATH` when set; discover missing paths below. All supporting paths are relative to this SKILL.md.

Inspect repository instructions, code/content branches, existing vehicle tooling, and running editor/server sessions. Locate the actual repositories; do not assume the last session's paths, ports, or running processes. Preserve unrelated servers and unsaved Blender work. Use a dedicated server for tests that change weather, synchronous mode, or world state.

Determine the vehicle's year, trim, wheel option, interior, intended camera distances, and traffic-density needs from the request and available context. State consequential assumptions. For fidelity work, retrieve and visually inspect real references before modeling: manufacturer dimensions plus front, rear, side, wheel, lamp, and cabin photographs. Distinguish nominal body width from mirror-inclusive width. Record URLs, provenance, licenses, and source hashes; inspection references are not automatically licensed textures. Describe licensed base-model improvements honestly.

## Choose the relevant workflow

- Geometry, wheels, surfaces, or visual critique: read [visual-authoring.md](references/visual-authoring.md).
- Unreal import, rigging, collision, paint, or lighting: read [carla-integration.md](references/carla-integration.md).
- Runtime demonstration, acceptance, or delivery: read [validation-delivery.md](references/validation-delivery.md).

For the proven reference implementation, locate `Util/VehicleAuthoring/TeslaModel3/` in the CARLA code repo. Read its README and the relevant scripts before adapting them. It is a vehicle-specific reference implementation, not a generic importer: model names, geometry selections, physical values, paths, and test assumptions need adaptation. If unavailable, use the guidance here and the target branch's actual APIs.

## Work in reviewable increments

Keep an unposed export master separate from posed inspection copies. Preserve reproducible source-generation/export steps, import settings, and asset identities. Establish correct scale, silhouette, wheel anchors, and basic runtime behavior before investing in fine detail. Review each meaningful visual increment in Blender and CARLA under matched camera and lighting conditions.

Debug the full chain: source geometry → exported geometry/material slots → saved Unreal assets → runtime components/state → sensor image. Do not assume a successful import, an API state round-trip, or a plausible render proves the next layer works. Reopen saved assets in a fresh process when persistence matters.

End with the editable source, integrated assets, reproducible tooling, provenance, and evidence appropriate to the requested scope. State remaining fidelity, dynamics, and performance limits. Commit/push only within the user's authorization; asset creation alone does not request publication.
