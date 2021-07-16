# Customizing maps: Traffic Lights and Signs

This guide explains how to add traffic lights and signs to your custom map, configure the area of influence of each one, and how to configure traffic lights as a group at junctions. This option is only available to users who have access to the Unreal Engine editor.

- [__Traffic Lights__](#traffic-lights)
- [__Traffic signs__](#traffic-signs)
- [__Next steps__](#next-steps)

---

## Traffic lights

To add traffic lights to your new map:

__1.__ From the _Content Browser_, navigate to `Content/Carla/Static/TrafficLight/StreetLights_01`. You will find several different traffic light blueprints to choose from.

__2.__ Drag the traffic lights into the scene and position them in the desired location. Press the space bar on your keyboard to toggle between positioning, rotation, and scaling tools.

__3.__ Adjust the [`trigger volume`][triggerlink] for each traffic light by selecting the _BoxTrigger_ component in the _Details_ panel and adjusting the values in the _Transform_ section. This will determine the traffic light's area of influence.

>>![ue_trafficlight](../img/ue_trafficlight.jpg)

__4.__ For junctions, drag the `BP_TrafficLightGroup` actor into the level. Assign all the traffic lights in the junction to the traffic light group by adding them to the _Traffic Lights_ array in the _Details_ panel.

>>![ue_tl_group](../img/ue_tl_group.jpg)

__5.__ Traffic light timing is only configurable through the Python API. See the documentation [here](core_actors.md#traffic-signs-and-traffic-lights) for more information.

>>![ue_tlsigns_example](../img/ue_tlsigns_example.jpg)

> _Example: Traffic Signs, Traffic lights and Turn based stop._

[triggerlink]: python_api.md#carla.TrafficSign.trigger_volume

---

## Traffic signs

To add traffic lights to your new map:

__1.__ From the _Content Browser_, navigate to `Content/Carla/Static/TrafficSign`. You will find several different traffic light blueprints to choose from.

__2.__ Drag the traffic lights into the scene and position them in the desired location. Press the space bar on your keyboard to toggle between positioning, rotation, and scaling tools.

__3.__ Adjust the [`trigger volume`][triggerlink] for each traffic sign by selecting the _BoxTrigger_ component in the _Details_ panel and adjusting the values in the _Transform_ section. This will determine the traffic light's area of influence. Not all traffic signs have a trigger volume. Those that do, include the yield, stop and speed limit signs.

---

## Next steps

Continue customizing your map using the tools and guides below:

- [Implement sub-levels in your map.](tuto_M_custom_layers.md)
- [Add buildings with the procedural building tool.](tuto_M_custom_buildings.md)
- [Customize the road with the road painter tool.](tuto_M_custom_road_painter.md)
- [Customize the weather](tuto_M_custom_weather_landscape.md#weather-customization)
- [Customize the landscape with serial meshes.](tuto_M_custom_weather_landscape.md#add-serial-meshes)

Once you have finished with the customization, you can [generate the pedestrian navigation information](tuto_M_generate_pedestrian_navigation.md).

---

If you have any questions about the process then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).
