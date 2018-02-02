Cameras and sensors
===================

Cameras and sensors can be added to the player vehicle by defining them in the
settings file sent by the client on every new episode. Check out the examples at
[CARLA Settings example][settingslink].

This document describes the details of the different cameras/sensors currently
available as well as the resulting images produced by them.

Although we plan to extend the sensor suite of CARLA in the near future, at the
moment there are only three different sensors available. These three sensors are
implemented as different post-processing effects applied to scene capture
cameras.

  * [Scene final](#scene-final)
  * [Depth map](#depth-map)
  * [Semantic segmentation](#semantic-segmentation)

!!! note
    The images are sent by the server as a BGRA array of bytes. The provided
    Python client retrieves the images in this format, it's up to the users to
    parse the images and convert them to the desired format. There are some
    examples in the PythonClient folder showing how to parse the images.

There is a fourth post-processing effect available, _None_, which provides a
view with of the scene with no effect, not even lens effects like flares or
depth of field; we will skip this one in the following descriptions.

We provide a tool to convert raw depth and semantic segmentation images to a
more human readable palette of colors. It can be found at
["Util/ImageConverter"][imgconvlink].

[settingslink]: https://github.com/carla-simulator/carla/blob/master/Docs/Example.CarlaSettings.ini

[imgconvlink]: https://github.com/carla-simulator/carla/tree/master/Util/ImageConverter

Scene final
-----------

![SceneFinal](img/capture_scenefinal.png)<br>

The "scene final" camera provides a view of the scene after applying some
post-processing effects to create a more realistic feel. These are actually
stored on the Level, in an actor called [PostProcessVolume][postprolink] and not
in the Camera. We use the following post process effects:

  * **Vignette** Darkens the border of the screen.
  * **Grain jitter** Adds a bit of noise to the render.
  * **Bloom** Intense lights burn the area around them.
  * **Auto exposure** Modifies the image gamma to simulate the eye adaptation to darker or brighter areas.
  * **Lens flares** Simulates the reflection of bright objects on the lens.
  * **Depth of field** Blurs objects near or very far away of the camera.

[postprolink]: https://docs.unrealengine.com/latest/INT/Engine/Rendering/PostProcessEffects/

Depth map
---------

![Depth](img/capture_depth.png)

The "depth map" camera provides an image with 24 bit floating precision point
codified in the 3 channels of the RGB color space. The order from less to more
significant bytes is R -> G -> B.

| R        | G        | B        | int24    |            |
|----------|----------|----------|----------|------------|
| 00000000 | 00000000 | 00000000 | 0        | min (near) |
| 11111111 | 11111111 | 11111111 | 16777215 | max (far)  |

Our max render distance (far) is 1km.

1. To decodify our depth first we get the int24.

        R + G*256 + B*256*256

2. Then normalize it in the range [0, 1].

        Ans / ( 256*256*256 - 1 )

3. And finally multiply for the units that we want to get. We have set the far plane at 1000 metres.

        Ans * far

Semantic segmentation
---------------------

![SemanticSegmentation](img/capture_semseg.png)

The "semantic segmentation" camera classifies every object in the view by
displaying it in a different color according to the object class. E.g.,
pedestrians appear in a different color than vehicles.

The server provides an image with the tag information **encoded in the red
channel**. A pixel with a red value of x displays an object with tag x. The
following tags are currently available

Value | Tag
-----:|:-----
   0  | None
   1  | Buildings
   2  | Fences
   3  | Other
   4  | Pedestrians
   5  | Poles
   6  | RoadLines
   7  | Roads
   8  | Sidewalks
   9  | Vegetation
  10  | Vehicles
  11  | Walls
  12  | TrafficSigns

This is implemented by tagging every object in the scene before hand (either at
begin play or on spawn). The objects are classified by their relative file path
in the project. E.g., every mesh stored in the
_"Unreal/CarlaUE4/Content/Static/Pedestrians"_ folder it's tagged as pedestrian.

!!! note
    **Adding new tags**:
    At the moment adding new tags is not very flexible and requires to modify
    the C++ code. Add a new label to the `ECityObjectLabel` enum in "Tagger.h",
    and its corresponding filepath check inside `GetLabelByFolderName()`
    function in "Tagger.cpp".
