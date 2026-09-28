# Tesla Model 3 authoring

Integrated and functionally validated in a CARLA UE5.8 test server. This is a reference-adjusted art candidate for visual review, not an OEM CAD/scan-verified asset or a validated Tesla dynamics model. See VALIDATION.md for evidence and limits.

Target: original-generation Model 3, 2018 Performance appearance (20-inch silver wheels and white interior). See SOURCES.md for attribution and the reference audit. The detailed mesh derives from Ameer Studio's CC BY asset; it was not modeled from scratch here.

The scripts prepare Blender source, independently articulated meshes, UE materials, wheel blueprints and a vehicle blueprint. The source mesh is pinned by SHA-256. `CARLA_MODEL3_WORKDIR` selects the working directory; the default is the workspace's `artifacts/tesla-model3` directory.

Dependencies: Python with requests, Blender, ImageMagick (`convert`), and the CARLA UE5.8 editor build. Execute from the workspace root:

```sh
python3 carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/fetch_source.py
blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/prepare_candidate.py
blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/export_parts.py
```

Run `import_unreal.py` and then `build_lods.py` using the editor's `-run=pythonscript -script=<absolute-script-path> -unattended -nullrhi` commandlet with the **absolute project path**. Build the native changes first: the USD importer save-path fix, safe door-constraint shutdown, and opt-in paint support for modular static panels. These are authoring operations that update this vehicle's assets. The import currently preserves existing imported meshes and the generated vehicle blueprint; set `CARLA_MODEL3_REIMPORT_PARTS=Body,DoorFL,...` to reimport selected geometry, then rebuild LODs. Blueprint attachment changes require deliberate regeneration.

Content destinations:

- `/Game/Carla/Static/Car/4Wheeled/TeslaModel3UE58`
- `/Game/Carla/Blueprints/Vehicles/TeslaModel3UE58/BP_TeslaModel3`
- API ID: `vehicle.tesla.model3`

The vehicle uses a single-speed EV approximation (1850 kg, 550 Nm peak, 9:1 ratio); this is not a validated Tesla dynamics model. The infotainment display is original static vector artwork, not a live infotainment simulation. Source photographs are inspection references and are not packaged as textures.

Runtime checks cover assembly, ground contact, acceleration, braking, wheel steering/spin, door articulation, light states, color variation and sensor bounds. Visual inspection must cover close-ups and LOD transitions. Actual test evidence is written to the work directory's `audit/` and `renders/` folders.

Run functional validation against a dedicated server (these scripts temporarily change weather and synchronous mode):

```sh
.venv-rtaov/bin/python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/validate_runtime.py --port 4674
.venv-rtaov/bin/python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/validate_cameras.py --port 4674
```

Modular paint uses the actor tag `Carla.StaticBodyworkPaint`, slots named `Bodywork_Mat`, and the clear-coat paint material's `Base Color` parameter. This opt-in preserves existing vehicles' Blueprint paint behavior. Door order follows the API (FL, FR, RL, RR). All four doors have simple convex collision; hood and trunk are separate meshes but are not animated by this implementation.

`Config/PostProcess/VehicleReview.json` is an optional neutral camera profile. Select it with the RGB sensor attribute `post_process_profile=VehicleReview`, or run `validate_cameras.py --profile VehicleReview`. It removes the default profile's saturation reduction, gamma grading and +1.2 EV exposure bias for material review; the default camera profile is unchanged. Keep default-profile captures when comparing ordinary CARLA output.

Calipers are separate meshes tagged `Carla.Caliper.0` through `.3` (FL, FR, RL, RR). The actor tag `Carla.ModularWheelCalipers` enables their wheel-centre/suspension and steering updates while excluding wheel rotation. Source hub meshes are exported independently; rim, tire and rotor geometry remains in the rotating wheel assembly.

Collision authoring must assign and save `SK_Model3Rig.physics_asset=PH_Model3`. A fresh-process check (`validate_assets.py`, run as an Unreal Python commandlet) guards against accidentally retaining the generic USD template physics asset. The native collision-copy helper invalidates cooked Chaos data after changing collision geometry. Wheel components use `NoCollision` and do not auto-weld; Chaos supplies wheel contact. The skeletal rig supplies bones and physics but is excluded from rendering, with bone refresh explicitly enabled.

The four wheel meshes currently retain LOD0 at every distance. Applying the body's percentage-based decimation to complete wheels damaged the much simpler tire surface while retaining excessive spoke detail, creating jagged tire silhouettes. This costs additional distant-wheel triangles; custom wheel LODs that preserve tire rings are a future optimization. Other mesh parts retain their four generated LODs.

Run `validate_collisions.py --port 4674` against a dedicated Town10HD_Opt server for rear, side and fixed-obstacle impacts at 30/60 Hz. It checks that collision sensors report contacts and measures peak speed and vertical displacement. It is a numerical stability regression, not an OEM crash-response validation.

### Wheel detail source

`enhance_wheels.py` reads the original master and writes `source/tesla-model3-wheel-detail.blend`. It replaces flat textured tires/discs with modeled tread channels, rounded sidewalls, raised inscriptions, vented discs and valve stems. It preserves the original spoke geometry and custom normals, replaces pre-lit rim/disc albedo with physical material colors, and creates tileable normal/roughness maps. The target remains the 20-inch Sport wheel with 235/35ZR20 Pilot Sport 4S appearance; tread and lettering are artist reconstructions rather than manufacturer tooling data.

Export this detailed version explicitly:

```sh
blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/enhance_wheels.py
CARLA_MODEL3_BLEND="$PWD/artifacts/tesla-model3/source/tesla-model3-wheel-detail.blend" blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/export_parts.py
```

Then run the Unreal import commandlet with `CARLA_MODEL3_REIMPORT_PARTS=WheelFL,WheelFR,WheelRL,WheelRR`. Normal maps use Blender/OpenGL orientation; the importer disables sRGB, uses normal-map compression and flips the green channel for Unreal. Roughness maps use linear masks. The wheels remain visual-only, with the existing physics radius and wheel anchors. Each detailed wheel has 324,782 triangles and retains full geometry at distance.

The tire close-up revision adds curved, embossed sidewall lettering, a dark branding panel, moulded annular ribs, finer angled tread cuts and 2048px rubber normal/roughness maps. `tesla-model3-tire-review.blend` is a posed inspection copy with the front wheel turned; export from `tesla-model3-wheel-detail.blend`, which retains straight wheel geometry. Reimport the four caliper meshes too when updating the painted TESLA lettering. Set `CARLA_MODEL3_REIMPORT_DETAIL_TEXTURES=1` to refresh the generated maps in an existing Unreal asset library.

The high-detail wheels prioritize close camera views and currently retain full geometry at distance. Fleet rendering cost still needs purpose-built wheel LODs before this detail level is used broadly in traffic.

### Surface and construction upgrade

The current upgrade pipeline continues from the straight wheel source:

```sh
blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/upgrade_surfaces.py
blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/upgrade_details.py
blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/upgrade_optics.py
CARLA_MODEL3_BLEND="$PWD/artifacts/tesla-model3/source/tesla-model3-optics-upgrade.blend" blender --background --python carla-ue58-dev/Util/VehicleAuthoring/TeslaModel3/export_parts.py
```

Reimport Body, all four doors, Hood and Trunk; all materials are updated by the
importer. Set `CARLA_MODEL3_REIMPORT_DETAIL_TEXTURES=1` to refresh generated maps.

The subsequent optical pass is `upgrade_optics.py`, reading the detail master and
writing `source/tesla-model3-optics-upgrade.blend`. It replaces baked chrome
reflections with metallic materials and original flute normals, and refines the
reflector bowls. Export that source with `CARLA_MODEL3_BLEND`, reimport `Body` and
the detail textures, then rebuild LODs. The optical revision has been reviewed in
Blender and CARLA; see `UPGRADE.md` for acceptance evidence.

`capture_upgrade.py --port 4674 --light-audit` captures ten isolated API states
from six fixed views and checks the reported vehicle state after each capture.
The white upper strips have separate Position materials; the roof lamp uses
Interior. Headlamp emitter brightness follows the maximum of low beam and twice
high beam. Seven authored light components illuminate the road/cabin and use
`Carla.Light.<group>` tags. Native visibility control runs through the saved light
defaults path, so this vehicle retains its `BP_TeslaModel3_C` defaults entry.
Beam cones and intensities are visual approximations, not measured photometry.
Run `build_lods.py`, `validate_assets.py` and `validate_surface_assets.py` as
Unreal commandlets afterward. On the dedicated server, run `validate_runtime.py`,
`capture_upgrade.py` and `validate_collisions.py`; each accepts `--port 4674`.

Rear indicator/reverse surfaces are split by vehicle side and share one material
per side. `Reverse` selects white; the matching `Left Blinker` or `Right Blinker`
selects amber with priority when both channels are requested. This avoids the
source model's overlapping opaque faces hiding the indicator. Both rear camera
angles are included in `capture_upgrade.py --light-audit`.

Rear material generation asserts the unnamed Saturate/OneMinus pin connections.
`validate_surface_assets.py` checks those saved connections, since merely finding
scalar parameters does not establish that the emissive shader is connected. Both
runtime validation and camera review drain restored-weather callbacks before
client teardown.

The reviewed editable Blender master is tracked with Git LFS in the content
repository at `Static/Car/4Wheeled/TeslaModel3UE58/Source/TeslaModel3.blend`.
It corresponds to the optics-upgrade source hash in `validation.json`. Copy it
to the work directory or select it with `CARLA_MODEL3_BLEND` when exporting.

Paired content revision (`ue58-dev-carla`): `8b34717330184ff70c4ff7893676f0dfb5f389d4`.
Use this CARLA code revision with that content revision or a descendant.
