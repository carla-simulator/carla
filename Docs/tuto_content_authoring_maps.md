# Content authoring - Maps

CARLA comes with a generous compliment of assets for creating driving simulations out of the box. However, the real power of CARLA comes in its comprehensive extensibility, allowing users to create entirely custom environments populated with bespoke assets like buildings, benches, trash cans, statues, street lights and bus stops. 

In this tutorial we will cover the process of creating a simple map for use with CARLA. We will use two software packages to create parts of the map. We will create the road network using [__RoadRunner__](https://es.mathworks.com/products/roadrunner.html) and then add assets to the map through the [__Unreal Editor__](https://www.unrealengine.com/en-US/features/the-unreal-editor).

## Create a road network using RoadRunner

Open RoadRunner and create a new scene. Choose the Road Plan Tool and right click in the workspace to drop the first control point for the road. Click and drag elswhere in the workspace to extend the road. 

[roadrunner_draw!](img/tuto_content_authoring_maps/drawing_roads.gif)

