# Generating Maps in RoadRunner

RoadRunner is the recommended software to create maps to be imported into CARLA. This guide outlines what RoadRunner is, things to consider when building the map and how to export custom maps ready for importing into CARLA. 

- [__Introduction to RoadRunner__](#introduction-to-roadrunner)
- [__Before you start__](#before-you-start)
- [__Build a map in RoadRunner__](#build-a-map-in-roadrunner)
- [__Export a map in RoadRunner__](#export-a-map-in-roadrunner)
- [__Next steps__](#next-steps)
---
## Introduction to RoadRunner

RoadRunner is an interactive editor that lets you design 3D scenes for simulating and testing automated driving systems. It can be used to create road layouts and accompanying OpenDRIVE and geometry information. Find out more about RoadRunner [here][rr_home].

RoadRunner is part of the MATLAB Campus-Wide Licenses, so many universities can provide unlimited academic access. [Check][rr_eligibility] if your university has access. Reach out to *automated-driving@mathworks.com* for any questions or troubles regarding accessibility. There is also a [trial version][rr_trial_version] available.

A license for RoadRunner is also available to everyone participating in the CARLA Leaderboard. Click [here][rr_leaderboard] for more information.

[rr_home]: https://www.mathworks.com/products/roadrunner.html
[rr_trial_version]: https://www.mathworks.com/products/roadrunner.html
[rr_eligibility]: https://www.mathworks.com/academia/tah-support-program/eligibility.html
[rr_leaderboard]: https://www.mathworks.com/academia/student-competitions/carla-autonomous-driving-challenge.html

---
## Before you start 

You will need to install RoadRunner. You can follow the [installation guide][rr_docs] at the Mathworks website.   

[rr_docs]: https://www.mathworks.com/help/roadrunner/ug/install-and-activate-roadrunner.html

---

## Build a map in RoadRunner

The specifics of how to build a map in RoadRunner go beyond the scope of this guide, however, there are video tutorials available in the [RoadRunner documentation][rr_tutorials].

__Keep in mind that a map heavy with props can slow the import process significantly.__ This is because Unreal Engine needs to convert every mesh to an Unreal asset. If you plan to import your map into a source build version of CARLA, we highly recommend that you only create the road layout in RoadRunner and leave any customization until after the map has been imported into Unreal Engine. CARLA provides several tools that you can use in the Unreal Engine editor to simplify the customization process.

---

## Export a map in RoadRunner

[rr_tutorials]: https://www.mathworks.com/support/search.html?fq=asset_type_name:video%20category:roadrunner/index&page=1&s_tid=CRUX_topnav

Below is a basic guideline to export your custom map from RoadRunner. You can find more detailed information about how to export to CARLA in [MathWorks' documentation][exportlink].

[exportlink]: https://www.mathworks.com/help/roadrunner/ug/Exporting-to-CARLA.html

Once you have made your map in RoadRunner you will be able to export it. Be aware that __the road layout cannot be modified after it has been exported.__ Before exporting, ensure that:

- The map is centered at (0,0) to ensure the map can be visualized correctly in Unreal Engine.
- The map definition is correct.
- The map validation is correct, paying close attention to connections and geometries.


>>>>![CheckGeometry](../img/check_geometry.jpg)

Once the map is ready, click on the `OpenDRIVE Preview Tool` button to visualize the OpenDRIVE road network and give everything one last check.

>>>>![checkopen](../img/check_open.jpg)

!!! note
    _OpenDrive Preview Tool_ makes it easier to test the integrity of the map. If there are any errors with junctions, click on `Maneuver Tool`, and `Rebuild Maneuver Roads`.

When you are ready to export:

__1.__ Export the scene using the CARLA option:

  - In the main toolbar, select `File` -> `Export` -> `CARLA (.fbx, .xodr, .rrdata.xml)`

__2.__ In the window that pops up:

>- Check the following options:
    - _Split by Segmentation_: Divides the mesh by semantic segmentation.
    - _Power of Two Texture Dimensions_: Improves performance.
    - _Embed Textures_: Ensures textures are embedded in the mesh.
    - _Export to Tiles_: Choose the size of the tile or leave unchecked for only one piece.

>- Leave unchecked:
    - _Export Individual Tiles_: Generates one `.fbx` file with all map pieces. 

>>>>![roadrunner_export](../img/roadrunner_export.png)

__3.__ Choose the directory where you want to export your files and click `Export`. This will generate `<mapName>.fbx` and `<mapName>.xodr` files among others. 
  
!!! Warning
    Make sure that the `.xodr` and the `.fbx` files have the same name.  

---

## Next steps

You are now ready to import your map into CARLA. The next step will depend upon the kind of CARLA installation you are using:

* __For users of CARLA built from source__, follow the guide [__here__](tuto_M_add_map_source.md).
* __For users of a packaged (binary) version of CARLA__, follow the guide [__here__](tuto_M_add_map_package.md).

---

If you have any questions about the process, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>