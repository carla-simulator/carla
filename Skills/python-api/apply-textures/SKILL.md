---
name: apply-textures
description: Replaces the textures of map objects (buildings, props, roads) on a running CARLA server through texture streaming — lists the object names the texture calls accept, pushes an image as the diffuse, normal, packed AO/roughness/metallic/emissive or emissive map of one object, a list, or every object whose name matches, or paints them a flat colour — and verifies the change by rendering the object with a temporary camera before and after and reporting the share of pixels that changed. Use when the user asks to "change the texture of a building", "repaint this facade", "put my image on that wall", "make the apartment red", "randomise building textures for domain randomisation", or "which objects can I retexture".
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Needs Pillow to read images, and numpy for --verify. Verified against ue58-dev ccc52d24d (CARLA 1.0) on Town10HD_Opt, editor -game server.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/textures.md
---

# Apply textures to map objects

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

The texture calls take an object's name as the server knows it, and change every material on that object that exposes the chosen parameter. A change lasts until the world is reloaded.

Hiding objects belongs to `toggle-env-objects`; recolouring vehicles is a blueprint attribute at spawn (`spawn-vehicles`).

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Find the object name (list --match)
- [ ] Step 3: Apply with --verify, and read the changed-pixel share
- [ ] Step 4: Look at the saved frame
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2: Find the object

```bash
source scripts/env.sh
python3 scripts/textures.py list --match apartment
```

`list` prints names from `world.get_names_of_all_objects()`, 2131 on Town10HD_Opt. `get_environment_objects()` names mesh components instead (`BP_Apartment04_v07_Opt_0_SM_1`), which the texture calls do not accept.

### Step 3: Apply

```bash
T="python3 scripts/textures.py"
$T solid --object BP_Apartment04_v07_Opt_0 --color 255,0,0 --verify
$T apply --object BP_Apartment04_v07_Opt_0 --diffuse facade.png --verify
$T apply --object BP_Apartment04_v07_Opt_0 --normal facade_n.png --orme facade_orme.png
$T apply --match apartment04_v06 --diffuse facade.png         # every object whose name matches
```

| Flag | Parameter | Texture type sent |
|---|---|---|
| `--diffuse` | `Diffuse` | `TextureColor`, 8-bit RGBA |
| `--normal` | `Normal` | `TextureFloatColor` |
| `--orme` | `AO_Roughness_Metallic_Emissive` (R, G, B, A) | `TextureFloatColor` |
| `--emissive` | `Emissive` | `TextureFloatColor`, scaled by `--emissive-scale` (5) |

Images larger than `--max-size` (1024) are downscaled first. `--match` refuses more than `--max-objects` (20) objects, so a broad pattern does not repaint half the map by accident.

### Step 4: Verify

`--verify` spawns a 640×480 RGB camera above and to the side of the first object, grabs a frame before and after, prints the share of pixels that changed by more than 12 levels, saves the after frame (`--verify-out`), and destroys the camera. Under 0.5% means the material did not take the parameter. Open the saved frame to check it is the right building.

## Gotchas

- **Only parameters the material exposes change.** Each `MaterialParameter` writes a fixed set of material parameter names (`Docs/tuto_texture_streaming.md` lists them). CARLA's generic building material takes all four; the road material takes only Diffuse and Normal. A material with none of the names keeps its look, without an error.
- **Emissive is faint by day.** A full-yellow emissive map ×5 changed 14.6% of the pixels on a sunlit apartment block, against 38.6% for a diffuse checkerboard. Test emissive maps at night (`set-weather`).
- **A wrong name fails loudly.** The server raises `unable to find Actor to apply the texture`; the script checks names first and lists the missing ones.
- **The change does not survive a reload.** `client.reload_world()` brings the original textures back.

## Examples

**Example 1: a red building**

User says: "make that apartment block bright red"

`list --match apartment`, pick the name, `solid --object <name> --color 255,0,0 --verify`.

**Example 2: domain randomisation**

User says: "give every apartment a random facade before each episode"

Call `apply --object <name> --diffuse <random image>` per building, from a loop over `list --match apartment`; reload the world to reset.

## Troubleshooting

**Problem: `verify: almost nothing changed`**
Cause: the material lacks the parameter, the camera framed the wrong side, or the texture is the same as before.
Solution: try `--diffuse`, open the saved frame, check the name.

**Problem: `--verify: no environment object named <name>_SM_*`**
Cause: the object has no static mesh components listed by `get_environment_objects()`.
Solution: drop `--verify` and check with a camera of your own (`create-sensor`).

## Outputs

Changes textures on the server until the next reload. `--verify` writes one PNG and leaves no actor behind.

API details, timings and the measured runs are in [references/textures.md](references/textures.md).
