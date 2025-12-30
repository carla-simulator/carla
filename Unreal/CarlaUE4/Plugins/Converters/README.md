NVIDIA Unreal Engine SimReady OpenUSD and MDL Converter Plugins
===============================================================
The SimReady OpenUSD and MDL Converters provide export and import support for SimReady OpenUSD stages and MDL materials.

- [SimReady documentation](https://docs.omniverse.nvidia.com/simready/latest/overview.html)
- [MDL documentation](https://docs.omniverse.nvidia.com/materials-and-rendering/latest/mdl_language.html)

Software Licenses
-----------------
This software is distributed under the [MIT License](./LICENSE.md)

The SimReady Converter plugins use the following software:
- NVIDIA MDL SDK [BSD-3-Clause License](./MDL/Licenses/MDL-SDK-LICENSE.md)
- NVIDIA Omniverse Core Material Library [BSD-3-Clause License](./MDL/Licenses/OMNIVERSE-CORE-MATERIALS-LICENSE.md)
- NVIDIA Texture Tools SDK [License](./SimReady/Licenses/NVTT-LICENSE.TXT)

Basic SimReady Converter Plugin Usage
-------------------------------------
- Export a map
  - Open a map (Town10HD_Opt is supported, but takes 5-10 minutes if exporting materials, smaller maps export faster)
  - Select `File > Export All`
  - Set `Save as type:` to USD binary or text
  - Choose a folder to save the exported USD Stage
  - Use the default settings to export
    - If using an RTX renderer like Omniverse Kit, `Include MDL` will export Unreal materials as MDL materials
    - If using USDView, `Include USD Preview Surface` may be selected instead
- Export a map with a commandlet
  - `./UE4Editor-Cmd ${CARLA_ROOT}/Unreal/CarlaUE4/CarlaUE4.uproject -run=SimReadyExport -AllowCommandletRendering -map=Town10HD_Opt.umap -path=${USD_OUTPUT} -MDL=true -log`
  - If running this commandlet in automation, it may be useful to add these arguments (depending on your setup): `-Unattended -NoSound -RenderOffScreen`
- Export a vehicle
  - Find a vehicle blueprint in the Content Browser, like `BP_Ford_Crown`
  - Right click on the blueprint and select `SimReady > Export as USD`
  - Choose a folder to save the exported vehicle as a USD Stage
  - Use the default settings to export unless you want USD Preview Surface materials
- Export a pedestrian
  - Find a pedestrian blueprint in the Content Browser, like `BP_Walker_MakeAmer_v2`
  - Right click on the blueprint and select `SimReady > Export as USD`
  - Choose a folder to save the exported pedestrian as a USD Stage
  - Use the default settings to export unless you want USD Preview Surface materials
- Converting MDL Materials
  - When converting Unreal materials to MDL there are many material functions that are not supported, like screen space, object space, or camera effects. These will produce warnings when exporting. Sometimes these issues create significant differences between renderers, other times the differences are negligible.
  - There is a Material Reparenting system (a slightly different version documented [here](https://docs.omniverse.nvidia.com/connect/latest/ue4/manual.html#material-reparenting-for-omniverse-core-materials)). It allows users to map Unreal material parameters to MDL core material parameters. Example core materials are OmniPBR, OmniGlass, SimPBR, and more.