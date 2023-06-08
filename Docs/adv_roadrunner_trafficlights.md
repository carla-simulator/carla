# Spawning traffic lights in RoadRunner

CARLA requires that traffic lights are spawned in RoadRunner in a specific way to ensure that the traffic lights will correctly spawn when the map loaded in CARLA. Roadrunner has a number of traffic light assets that can be used for creating maps internally. However only one option works for maps intended to be imported into CARLA - the Signal_3Light_Post01 asset.

Follow these steps to spawn traffic lights in RoadRunner that will work in CARLA:

- Select the Signal_3Light_Post01 signal in the library browser. You can find it in `Assets/Props/Signals`.

![rr_traffic_light](../img/rr_traffic_light.png)

- Select the signal tool.

![rr_signal_tool](../img/rr_signal_tool.png)

- Select the junction that you want to add signals to and press *Auto Signalize*

![rr_autosignalize](../img/rr_auto_signalize.png)

- In the new pop-up window select *Split Phase* and enable the *Automatically Place Selected Prop* checkbox. 

![rr_splitphase](../img/rr_split_phase.png)

- The previous step will signalize the selected prop. You may change the position and rotation of the prop if needed. 

![rr_autosignalize](../img/rr_auto_signalize.png)