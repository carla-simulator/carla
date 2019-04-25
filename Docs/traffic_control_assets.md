<h1>Traffic Controll Assets</h1>

These are the blueprints in charge of controlling the vehicle population while they are not controlled by a client or another person.

<h2>TrafficLights/TrafficLightGroup Actors</h2> 
Distributes which lanes enter into any crossroad section. TrafficLightGroup stores any number of traffic light and iterates which one of them is currently on its green state. (only one at a time).

<h3>TrafficLight</h3> 
Once Spawned in the world or created a child of the actor inside the content browser you might setup the lights as you want. Start by adding to the Heads Variable as much items as you want to have you might then, name them set their position and configure the lights used. Please, notice that as for today, all of the heads share the same State. So adding more heads is only for Aesthetic purposes.

 By default each head created will have three lights: red in the bottom, a Yellow one in the middle and a green one in the top. Those might be reconfigured and lights might be added or subtracted (By adding items to the Lights Array).

 Each light consist of the following parameters: 

   - Mesh: The lightMesh
   - Relative transform: Transform relative to the head position.
   - LightColor: Light of the color.
   - Masked: Whether if the light has a mask or not (Arrow, cross, pedestrian...)
   - Mask: The Mask to be used.
   - Rotation: If the light uses a mask, rotation of the mask.


Once these are configured you might place it wherever you need. Once in the world you'll need to check the BoxTrigger position, Make sure it is located where you need your vehicles to stop before the lights.


<h3>TrafficLightGroup</h3>

Once the traffic lights are placed in the crossing you'll need to create a trafficlightGroup to distribute which of the traffic light is currently on. Create an Item to the trafficlights array for each light in the crossing and assign it.

Once every light of the intersection is assigned it is time to configure the timing:

  - Green time: How much time each light is set to green.
  - Yellow time: How much time it spends on yellow before turning red.
  - Wait time: How much time to wait before making the next traffic light in the list green.


!!!Note
  
  It's easy to write different simple controls for the traffic lights if you need an specific sequence of light changes. In blueprints store the reference to the Traffic light to control and call "Set Traffic Light State" whenever you need to change the light state.


<h3>SpeedLimiters</h3>

The only ones available right now are the speed limiters. These give a signal to the vehicle that crosses over its trigger box to circulate at the speed defined on it. You can create as many exit routes as you need and once created you can choose the percentage of any of the routes of being taken by the vehicle.


<h3>RoutePlanner</h3>

Route planners are composed of a trigger box and a number of Splines. Once any vehicle crosses the trigger it choses any of the routes available at random and follows it. Each route is represented with a Spline. The vehicle will follow each point of the spline in order until the spline ends. Each spline has assigned a chance of being chosen by the car.


<h3>StopSign</h3>

It's composed of two boxes, the checker box counts how many vehicles are inside of it and While this number is greater than 0 stops any vehicle trying to get through the "Vehicle Stopper" trigger. Place it in the world and set the two boxes wherever you need. Be careful not to create a situation in which two vehicles are waiting for each other to move.


<h3>TurnbasedStopSigns</h3>

For 4-Stops intersections. This stop trigger registers all vehicles that enter on it and stops them, releasing them in entrance order once any vehicle leaves the crossing. It's a single prism trigger. cover all the intersection with it, making sure the limits of the volume coincide with the stop marking lines. 
