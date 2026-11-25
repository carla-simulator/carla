# Texture streaming reference

## API

| Call | Texture |
|---|---|
| `world.get_names_of_all_objects()` | the names the calls below accept |
| `world.apply_color_texture_to_object(name, MaterialParameter, TextureColor)` | 8-bit RGBA |
| `world.apply_float_color_texture_to_object(name, MaterialParameter, TextureFloatColor)` | float RGBA, HDR allowed |
| `world.apply_textures_to_object(name, diffuse, emissive, normal, orme)` | all four at once; an empty texture is skipped |
| `..._to_objects(names, ...)` | the same for a list of names, in one call |

`carla.TextureColor(w, h)` and `carla.TextureFloatColor(w, h)` have no bulk setter: the client fills them with one `set(x, y, colour)` per pixel. `textures.py` reads pixels through `Image.load()` and downscales to `--max-size` first.

`CarlaGameModeBase.h` marks the server's old name-lookup helper deprecated ("the apply_textures API relies on IDs"), but the Python calls still take names on ue58-dev ccc52d24d.

## Material parameters written

From `Docs/tuto_texture_streaming.md`:

| `MaterialParameter` | Material parameter names |
|---|---|
| `Diffuse` | `BaseColor`, `Difuse`, `Difuse 2`, `Difuse 3`, `Difuse 4` |
| `Normal` | `Normal`, `Normal 2`, `Normal 3`, `Normal 4` |
| `AO_Roughness_Metallic_Emissive` | `AO / Roughness / Metallic / Emissive`, `ORMH`, `ORMH 2`, `ORMH 3`, `ORMH 4` |
| `Emissive` | `Emissive` |

## Measured on ue58-dev ccc52d24d, Town10HD_Opt, editor -game server

| Run | Result |
|---|---|
| `list` | 2131 paintable objects, 16 matching `apartment` |
| `solid --color 255,0,0` on `BP_Apartment04_v07_Opt_0` | 64.2% of the verify camera's pixels changed |
| 512×512 checkerboard as diffuse | 38.6% changed; the whole block, fire escapes included, shows the pattern |
| 1024×1024 yellow as emissive ×5, daytime | 14.6% changed, barely visible |
| 2048×2048 flat normal map, downscaled to 1024 | 16.1% changed; built and sent in 0.8 s |
| `--match apartment04_v06` | 3 objects |
| `client.reload_world()` afterwards | original textures back (96.1% of pixels changed from the painted frame) |
