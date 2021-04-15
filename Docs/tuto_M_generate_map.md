# Generating Maps in RoadRunner

RoadRunner is the recommended software to create maps to be imported into CARLA. This guide will give an outline of what RoadRunner is and how to export customs maps that are ready for importing into CARLA.

- [__Introduction to Roadrunner__](#introduction-to-roadrunner)
- [__Before you start__](#before-you-start)
- [__Export a map in RoadRunner__](#export-a-map-in-roadrunner)
- [__Next steps__](#next-steps)
---
## Introduction to RoadRunner

RoadRunner is an interactive editor that lets you design 3D scenes for simulating and testing automated driving systems. You can customize roadway scenes by creating region-specific road signs and markings. You can insert signs, signals, guardrails, and road damage, as well as foliage, buildings, and other 3D models. RoadRunner provides tools for setting and configuring traffic signal timing, phases, and vehicle paths at intersections. the  Asset Library lets you quickly populate your 3D scenes with a large set of realistic and visually consistent 3D models.

RoadRunner is part of the MATLAB Campus Wide Licenses, so many universities can provide unlimited academic access. [Check][rr_eligibility] if your university has access. Reach out to *automated-driving@mathworks.com* for any questions or troubles regarding accessibility. There is also a [trial version][rr_trial_version] available.

[rr_trial_version]: https://www.mathworks.com/products/roadrunner.html
[rr_eligibility]: https://www.mathworks.com/academia/tah-support-program/eligibility.html

---
## Before you start 

__1.__ Install RoadRunner. You can follow the [installation guide][rr_docs] at the Mathworks website.   

[rr_docs]: https://www.mathworks.com/help/roadrunner/ug/install-and-activate-roadrunner.html

---
## Export a map in RoadRunner

!!! Note
    The specifics of how to build a map in RoadRunner go beyond the scope of this guide, however there are video tutorials available in the [RoadRunner documentation][rr_tutorials].

[rr_tutorials]: https://www.mathworks.com/support/search.html?fq=asset_type_name:video%20category:roadrunner/index&page=1&s_tid=CRUX_topnav

Below is a guideline to export your custom map from RoadRunner. You can find more detailed information about how to export to CARLA in [MathWorks' documentation][exportlink].

[exportlink]: https://www.mathworks.com/help/roadrunner/ug/Exporting-to-CARLA.html

Once you have made your map in Roadrunner you will be able to export it. Be aware that __the map cannot be modified after it has been exported.__ Before exporting, ensure that:

- The map is centered at (0,0) to ensure the map can be visualized correctly in Unreal Engine.
- The map definition is correct.
- The map validation is correct, paying close attention to connections and geometries.


![CheckGeometry](img/check_geometry.jpg)

Once the map is ready, click on the `OpenDRIVE Preview Tool` button to visualize the OpenDRIVE road network and give everything one last check.

![checkopen](img/check_open.jpg)

!!! note
    _OpenDrive Preview Tool_ makes it easier to test the integrity of the map. If there are any errors with junctions, click on `Maneuver Tool`, and `Rebuild Maneuver Roads`.


__1.__ Export the scene using the CARLA option:

  - In the main toolbar, select `File` -> `Export` -> `CARLA (.fbx, .xodr, .rrdata.xml)`

__2.__ Check the following options:

- Split by Segmentation: Divides the mesh by semantic segmentation.
- Power of Two Texture Dimensions: Improves performance.
- Embed Textures: Ensures textures are embedded in the mesh.
- Export to Tiles: Choose the size of the tile or leave unchecked for only one piece.

Leave unchecked:

- `Export Individual Tiles`: Generates one `.fbx` file with all map pieces. 

![roadrunner_export](/img/roadrunner_export.png)

__3.__ Chose the directory where you want to export your files and click `Export`. This will generate `<mapName>.fbx` and `<mapName>.xodr` files among others. 
  
!!! Warning
    Make sure that the `.xodr` and the `.fbx` files have the same name.  

---

## Next steps

You are now ready to import your map into CARLA. The next step will depend upon the kind of CARLA installation you are using:

* __For users of CARLA built from source__, follow the guide [__here__](tuto_M_add_map_source.md).
* __For users of a packaged (binary) version of CARLA__, follow the guide [__here__](tuto_M_add_map_package.md).
