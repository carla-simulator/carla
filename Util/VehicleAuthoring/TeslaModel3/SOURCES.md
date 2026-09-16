# Model 3 sources and status

The detailed development asset derives from **Tesla 2018 Model 3** by **Ameer Studio** (uchiha.321abc):
https://sketchfab.com/3d-models/tesla-2018-model-3-5ef9b845aaf44203b6d04e2c677e444f
https://sketchfab.com/uchiha.321abc

Published license: **Creative Commons Attribution 4.0 International**
https://creativecommons.org/licenses/by/4.0/

The public Sketchfab API metadata was retrieved on 2026-09-11 and retained in `reference/candidate-model-metadata.json`. The source GLB was obtained from the public AllenAI Objaverse dataset mirror. Source author is credited; this project does not claim original authorship of that geometry or its source textures.

Changes: metric/axis normalization against Tesla dimensions, wheel centers and radius correction, physical material response, glazing thickness, material consolidation, separation of parts for CARLA articulation, UE asset authoring, landscape screen proportion correction, independently articulated calipers and runtime validation.

Reference photographs are inspection-only, not shipped as project textures. See `reference/sources.json` and `reference/brief.md`. Selected appearance: 2018 Performance with 20-inch silver wheels and white interior. Matching owner photographs:
- https://www.findmyelectric.com/listings/2018-tesla-model-3-performance-128226/
- https://www.findmyelectric.com/listings/2018-tesla-model-3-performance-262445/

Manufacturer dimensions:
https://www.tesla.com/ownersmanual/2017_2023_model3/en_cn/GUID-56562137-FC31-4110-A13C-9A9FC6657BF0.html

Normalized targets: length 4694 mm, body width 1850 mm, height 1443 mm, wheelbase 2875 mm. These are approximate manufacturer dimensions; the asset is not CAD- or scan-verified. The infotainment texture embedded in the source depicted a Model S and was replaced with original static Model 3 display artwork.

The initial legacy-mesh reconstruction in `source/tesla-model3-development.blend` failed the visual-quality review and is not the selected asset. It is kept only as local development evidence.

Wheel detail revision references (2026-09-11):
- Tesla Model 3 manual, 20-inch Sport wheel illustration, 8.5-inch width and 235/35ZR20 Pilot Sport 4S fitment: https://www.tesla.com/ownersmanual/2017_2023_model3/en_us/GUID-ECA7C07B-7944-496B-8FC5-12762BF061F1.html
- Michelin product reference: https://michelinmedia.com/pilot-sport-4s/

The detail pass creates new tire/disc geometry, lettering, valve stems and synthetic material maps. No reference-photo pixels are used in these maps. The original Cup 2 sidewall texture and disc texture with baked spoke shadows are no longer assigned to these surfaces. Tread channels and lettering are visual approximations; neither reproduces manufacturer CAD or tooling.

Direct manufacturer close-up references inspected for the tire revision:
- https://michelinmedia.com/site/user/images/PS4S_Tread.jpg
- https://michelinmedia.com/site/user/images/PS4S_Sidewall_Top.jpg

Raised branding uses converted DejaVu Sans Bold Oblique outlines as an approximation, not the original Michelin logo artwork. The model does not include invented certification or serial markings.

Surface/construction revision: the additional reflector chambers, rear light-guide
thickness, arch liners/fasteners, undertray, window gaskets, steering stitches and
repeater lens geometry are authored approximations based on the retained visual
references and the source asset's fitting envelopes. They are not manufacturer
CAD. Upholstery and plastic normal/roughness maps are original procedural fields.
Body normal filtering changes shading without moving the source panel vertices.
