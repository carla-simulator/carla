#Python API reference
This reference contains all the details the Python API. To consult a previous reference for a specific CARLA release, change the documentation version using the panel in the bottom right corner.<br>This will change the whole documentation to a previous state. Remember that the <i>latest</i> version is the `dev` branch and may show features not available in any packaged versions of CARLA.<hr>  

## carla.AckermannControllerSettings<a name="carla.AckermannControllerSettings"></a>
Manages the settings of the Ackermann PID controller.  

### Instance Variables
- <a name="carla.AckermannControllerSettings.speed_kp"></a>**<font color="#f8805a">speed_kp</font>** (_float_)  
Proportional term of the speed PID controller.  
- <a name="carla.AckermannControllerSettings.speed_ki"></a>**<font color="#f8805a">speed_ki</font>** (_float_)  
Integral term of the speed PID controller.  
- <a name="carla.AckermannControllerSettings.speed_kd"></a>**<font color="#f8805a">speed_kd</font>** (_float_)  
Derivative term of the speed PID controller.  
- <a name="carla.AckermannControllerSettings.accel_kp"></a>**<font color="#f8805a">accel_kp</font>** (_float_)  
Proportional term of the acceleration PID controller.  
- <a name="carla.AckermannControllerSettings.accel_ki"></a>**<font color="#f8805a">accel_ki</font>** (_float_)  
Integral term of the acceleration PID controller.  
- <a name="carla.AckermannControllerSettings.accel_kd"></a>**<font color="#f8805a">accel_kd</font>** (_float_)  
Derivative term of the acceleration PID controller.  

### Methods
- <a name="carla.AckermannControllerSettings.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**speed_kp**=0.15</font>, <font color="#00a6ed">**speed_ki**=0.0</font>, <font color="#00a6ed">**speed_kd**=0.25</font>, <font color="#00a6ed">**accel_kp**=0.01</font>, <font color="#00a6ed">**accel_ki**=0.0</font>, <font color="#00a6ed">**accel_kd**=0.01</font>)  
    - **Parameters:**
        - `speed_kp` (_float_)  
        - `speed_ki` (_float_)  
        - `speed_kd` (_float_)  
        - `accel_kp` (_float_)  
        - `accel_ki` (_float_)  
        - `accel_kd` (_float_)  

##### Dunder methods
- <a name="carla.AckermannControllerSettings.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.AckermannControllerSettings](#carla.AckermannControllerSettings)</font>)  
- <a name="carla.AckermannControllerSettings.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.AckermannControllerSettings](#carla.AckermannControllerSettings)</font>)  
- <a name="carla.AckermannControllerSettings.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Actor<a name="carla.Actor"></a>
CARLA defines actors as anything that plays a role in the simulation or can be moved around. That includes: pedestrians, vehicles, sensors and traffic signs (considering traffic lights as part of these). Actors are spawned in the simulation by [carla.World](#carla.World) and they need for a [carla.ActorBlueprint](#carla.ActorBlueprint) to be created. These blueprints belong into a library provided by CARLA, find more about them [here](bp_library.md).  

### Instance Variables
- <a name="carla.Actor.attributes"></a>**<font color="#f8805a">attributes</font>** (_dict_)  
A dictionary containing the attributes of the blueprint this actor was based on.  
- <a name="carla.Actor.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Identifier for this actor. Unique during a given episode.  
- <a name="carla.Actor.type_id"></a>**<font color="#f8805a">type_id</font>** (_str_)  
The identifier of the blueprint this actor was based on, e.g. `vehicle.ford.mustang`.  
- <a name="carla.Actor.is_alive"></a>**<font color="#f8805a">is_alive</font>** (_bool_)  
Returns whether this object was destroyed using this actor handle.  
- <a name="carla.Actor.is_active"></a>**<font color="#f8805a">is_active</font>** (_bool_)  
Returns whether this actor is active (True) or not (False).  
- <a name="carla.Actor.is_dormant"></a>**<font color="#f8805a">is_dormant</font>** (_bool_)  
Returns whether this actor is dormant (True) or not (False) - the opposite of is_active.  
- <a name="carla.Actor.parent"></a>**<font color="#f8805a">parent</font>** (_[carla.Actor](#carla.Actor)_)  
Actors may be attached to a parent actor that they will follow around. This is said actor.  
- <a name="carla.Actor.semantic_tags"></a>**<font color="#f8805a">semantic_tags</font>** (_list(int)_)  
A list of semantic tags provided by the blueprint listing components for this actor. E.g. a traffic light could be tagged with `Pole` and `TrafficLight`. These tags are used by the semantic segmentation sensor. Find more about this and other sensors [here](ref_sensors.md#semantic-segmentation-camera).  
- <a name="carla.Actor.actor_state"></a>**<font color="#f8805a">actor_state</font>** (_[carla.ActorState](#carla.ActorState)_)  
Returns the [carla.ActorState](#carla.ActorState), which can identify if the actor is Active, Dormant or Invalid.  
- <a name="carla.Actor.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
Bounding box containing the geometry of the actor. Its location and rotation are relative to the actor it is attached to.  

### Methods
- <a name="carla.Actor.add_angular_impulse"></a>**<font color="#7fb800">add_angular_impulse</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**angular_impulse**</font>)  
Applies an angular impulse at the center of mass of the actor. This method should be used for instantaneous torques, usually applied once. Use __<font color="#7fb800">add_torque()</font>__ to apply rotation forces over a period of time.  
    - **Parameters:**
        - `angular_impulse` (_[carla.Vector3D](#carla.Vector3D)<small> - degrees*s</small>_) - Angular impulse vector in global coordinates.  
- <a name="carla.Actor.add_force"></a>**<font color="#7fb800">add_force</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**force**</font>)  
Applies a force at the center of mass of the actor. This method should be used for forces that are applied over a certain period of time. Use __<font color="#7fb800">add_impulse()</font>__ to apply an impulse that only lasts an instant.  
    - **Parameters:**
        - `force` (_[carla.Vector3D](#carla.Vector3D)<small> - N</small>_) - Force vector in global coordinates.  
- <a name="carla.Actor.add_impulse"></a>**<font color="#7fb800">add_impulse</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**impulse**</font>)  
Applies an impulse at the center of mass of the actor. This method should be used for instantaneous forces, usually applied once. Use __<font color="#7fb800">add_force()</font>__ to apply forces over a period of time.  
    - **Parameters:**
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)<small> - N*s</small>_) - Impulse vector in global coordinates.  
- <a name="carla.Actor.add_torque"></a>**<font color="#7fb800">add_torque</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**torque**</font>)  
Applies a torque at the center of mass of the actor. This method should be used for torques that are applied over a certain period of time. Use __<font color="#7fb800">add_angular_impulse()</font>__ to apply a torque that only lasts an instant.  
    - **Parameters:**
        - `torque` (_[carla.Vector3D](#carla.Vector3D)<small> - degrees</small>_) - Torque vector in global coordinates.  
- <a name="carla.Actor.destroy"></a>**<font color="#7fb800">destroy</font>**(<font color="#00a6ed">**self**</font>)  
Tells the simulator to destroy this actor and returns <b>True</b> if it was successful. It has no effect if it was already destroyed.  
    - **Return:** _bool_  
    - **Warning:** <font color="#ED2F2F">_This method blocks the script until the destruction is completed by the simulator.
_</font>  
- <a name="carla.Actor.disable_constant_velocity"></a>**<font color="#7fb800">disable_constant_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Disables any constant velocity previously set for a [carla.Vehicle](#carla.Vehicle) actor.  
- <a name="carla.Actor.enable_constant_velocity"></a>**<font color="#7fb800">enable_constant_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**velocity**</font>)  
Sets a vehicle's velocity vector to a constant value over time. The resulting velocity will be approximately the `velocity` being set, as with __<font color="#7fb800">set_target_velocity()</font>__.  
    - **Parameters:**
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)<small> - m/s</small>_) - Velocity vector in local space.  
    - **Note:** <font color="#8E8E8E">_Only [carla.Vehicle](#carla.Vehicle) actors can use this method.  
_</font>  
    - **Warning:** <font color="#ED2F2F">_Enabling a constant velocity for a vehicle managed by the [Traffic Manager](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/adv_traffic_manager/) may cause conflicts. This method overrides any changes in velocity by the TM.  
_</font>  

##### Getters
- <a name="carla.Actor.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's 3D acceleration vector the client recieved during last tick. The method does not call the simulator.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)<small> - m/s<sup>2</sup></small>_  
- <a name="carla.Actor.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's angular velocity vector the client recieved during last tick. The method does not call the simulator.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)<small> - deg/s</small>_  
- <a name="carla.Actor.get_location"></a>**<font color="#7fb800">get_location</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's location the client recieved during last tick. The method does not call the simulator.  
    - **Return:** _[carla.Location](#carla.Location)<small> - meters</small>_  
    - **Setter:** _[carla.Actor.set_location](#carla.Actor.set_location)_  
- <a name="carla.Actor.get_transform"></a>**<font color="#7fb800">get_transform</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's transform (location and rotation) the client recieved during last tick. The method does not call the simulator.  
    - **Return:** _[carla.Transform](#carla.Transform)_  
    - **Setter:** _[carla.Actor.set_transform](#carla.Actor.set_transform)_  
- <a name="carla.Actor.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's velocity vector the client recieved during last tick. The method does not call the simulator.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)<small> - m/s</small>_  
- <a name="carla.Actor.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Returns the world this actor belongs to.  
    - **Return:** _[carla.World](#carla.World)_  

##### Setters
- <a name="carla.Actor.set_enable_gravity"></a>**<font color="#7fb800">set_enable_gravity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**</font>)  
Enables or disables gravity for the actor. __Default__ is True.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Actor.set_location"></a>**<font color="#7fb800">set_location</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Teleports the actor to a given location.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)<small> - meters</small>_)  
    - **Getter:** _[carla.Actor.get_location](#carla.Actor.get_location)_  
- <a name="carla.Actor.set_simulate_physics"></a>**<font color="#7fb800">set_simulate_physics</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>)  
Enables or disables the simulation of physics on this actor.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Actor.set_target_angular_velocity"></a>**<font color="#7fb800">set_target_angular_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**angular_velocity**</font>)  
Sets the actor's angular velocity vector. This is applied before the physics step so the resulting angular velocity will be affected by external forces such as friction.  
    - **Parameters:**
        - `angular_velocity` (_[carla.Vector3D](#carla.Vector3D)<small> - deg/s</small>_)  
- <a name="carla.Actor.set_target_velocity"></a>**<font color="#7fb800">set_target_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**velocity**</font>)  
Sets the actor's velocity vector. This is applied before the physics step so the resulting angular velocity will be affected by external forces such as friction.  
    - **Parameters:**
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.set_transform"></a>**<font color="#7fb800">set_transform</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**transform**</font>)  
Teleports the actor to a given transform (location and rotation).  
    - **Parameters:**
        - `transform` (_[carla.Transform](#carla.Transform)_)  
    - **Getter:** _[carla.Actor.get_transform](#carla.Actor.get_transform)_  

##### Dunder methods
- <a name="carla.Actor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorAttribute<a name="carla.ActorAttribute"></a>
CARLA provides a library of blueprints for actors that can be accessed as [carla.BlueprintLibrary](#carla.BlueprintLibrary). Each of these blueprints has a series of attributes defined internally. Some of these are modifiable, others are not. A list of recommended values is provided for those that can be set.  

### Instance Variables
- <a name="carla.ActorAttribute.id"></a>**<font color="#f8805a">id</font>** (_str_)  
The attribute's name and identifier in the library.  
- <a name="carla.ActorAttribute.is_modifiable"></a>**<font color="#f8805a">is_modifiable</font>** (_bool_)  
It is <b>True</b> if the attribute's value can be modified.  
- <a name="carla.ActorAttribute.recommended_values"></a>**<font color="#f8805a">recommended_values</font>** (_list(str)_)  
A list of values suggested by those who designed the blueprint.  
- <a name="carla.ActorAttribute.type"></a>**<font color="#f8805a">type</font>** (_[carla.ActorAttributeType](#carla.ActorAttributeType)_)  
The attribute's parameter type.  

### Methods
- <a name="carla.ActorAttribute.as_bool"></a>**<font color="#7fb800">as_bool</font>**(<font color="#00a6ed">**self**</font>)  
Reads the attribute as boolean value.  
- <a name="carla.ActorAttribute.as_color"></a>**<font color="#7fb800">as_color</font>**(<font color="#00a6ed">**self**</font>)  
Reads the attribute as [carla.Color](#carla.Color).  
- <a name="carla.ActorAttribute.as_float"></a>**<font color="#7fb800">as_float</font>**(<font color="#00a6ed">**self**</font>)  
Reads the attribute as float.  
- <a name="carla.ActorAttribute.as_int"></a>**<font color="#7fb800">as_int</font>**(<font color="#00a6ed">**self**</font>)  
Reads the attribute as int.  
- <a name="carla.ActorAttribute.as_str"></a>**<font color="#7fb800">as_str</font>**(<font color="#00a6ed">**self**</font>)  
Reads the attribute as string.  

##### Dunder methods
- <a name="carla.ActorAttribute.__bool__"></a>**<font color="#7fb800">\__bool__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=bool / int / float / str / [carla.Color](#carla.Color) / [carla.ActorAttribute](#carla.ActorAttribute)</font>)  
Returns true if this actor's attribute and `other` are the same.  
    - **Return:** _bool_  
- <a name="carla.ActorAttribute.__float__"></a>**<font color="#7fb800">\__float__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__int__"></a>**<font color="#7fb800">\__int__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=bool / int / float / str / [carla.Color](#carla.Color) / [carla.ActorAttribute](#carla.ActorAttribute)</font>)  
Returns true if this actor's attribute and `other` are different.  
    - **Return:** _bool_  
- <a name="carla.ActorAttribute.__nonzero__"></a>**<font color="#7fb800">\__nonzero__</font>**(<font color="#00a6ed">**self**</font>)  
Returns true if this actor's attribute is not zero or null.  
    - **Return:** _bool_  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorAttributeType<a name="carla.ActorAttributeType"></a>
CARLA provides a library of blueprints for actors in [carla.BlueprintLibrary](#carla.BlueprintLibrary) with different attributes each. This class defines the types those at [carla.ActorAttribute](#carla.ActorAttribute) can be as a series of enum. All this information is managed internally and listed here for a better comprehension of how CARLA works.  

### Instance Variables
- <a name="carla.ActorAttributeType.Bool"></a>**<font color="#f8805a">Bool</font>**  
- <a name="carla.ActorAttributeType.Int"></a>**<font color="#f8805a">Int</font>**  
- <a name="carla.ActorAttributeType.Float"></a>**<font color="#f8805a">Float</font>**  
- <a name="carla.ActorAttributeType.String"></a>**<font color="#f8805a">String</font>**  
- <a name="carla.ActorAttributeType.RGBColor"></a>**<font color="#f8805a">RGBColor</font>**  

---

## carla.ActorBlueprint<a name="carla.ActorBlueprint"></a>
CARLA provides a blueprint library for actors that can be consulted through [carla.BlueprintLibrary](#carla.BlueprintLibrary). Each of these consists of an identifier for the blueprint and a series of attributes that may be modifiable or not. This class is the intermediate step between the library and the actor creation. Actors need an actor blueprint to be spawned. These store the information for said blueprint in an object with its attributes and some tags to categorize them. The user can then customize some attributes and eventually spawn the actors through [carla.World](#carla.World).  

### Instance Variables
- <a name="carla.ActorBlueprint.id"></a>**<font color="#f8805a">id</font>** (_str_)  
The identifier of said blueprint inside the library. E.g. `walker.pedestrian.0001`.  
- <a name="carla.ActorBlueprint.tags"></a>**<font color="#f8805a">tags</font>** (_list(str)_)  
A list of tags each blueprint has that helps describing them. E.g. `['0001', 'pedestrian', 'walker']`.  

### Methods
- <a name="carla.ActorBlueprint.has_attribute"></a>**<font color="#7fb800">has_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
Returns <b>True</b> if the blueprint contains the attribute `id`.  
    - **Parameters:**
        - `id` (_str_) - e.g. `gender` would return **True** for pedestrians' blueprints.  
    - **Return:** _bool_  
- <a name="carla.ActorBlueprint.has_tag"></a>**<font color="#7fb800">has_tag</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**tag**</font>)  
Returns <b>True</b> if the blueprint has the specified `tag` listed.  
    - **Parameters:**
        - `tag` (_str_) - e.g. 'walker'.  
    - **Return:** _bool_  
- <a name="carla.ActorBlueprint.match_tags"></a>**<font color="#7fb800">match_tags</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Returns <b>True</b> if any of the tags listed for this blueprint matches `wildcard_pattern`. Matching follows [fnmatch](https://docs.python.org/2/library/fnmatch.html) standard.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _bool_  

##### Getters
- <a name="carla.ActorBlueprint.get_attribute"></a>**<font color="#7fb800">get_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
Returns the actor's attribute with `id` as identifier if existing.  
    - **Parameters:**
        - `id` (_str_)  
    - **Return:** _[carla.ActorAttribute](#carla.ActorAttribute)_  
    - **Setter:** _[carla.ActorBlueprint.set_attribute](#carla.ActorBlueprint.set_attribute)_  

##### Setters
- <a name="carla.ActorBlueprint.set_attribute"></a>**<font color="#7fb800">set_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>, <font color="#00a6ed">**value**</font>)<button class="SnipetButton" id="carla.ActorBlueprint.set_attribute-snipet_button">snippet &rarr;</button>  
If the `id` attribute is modifiable, changes its value to `value`.  
    - **Parameters:**
        - `id` (_str_) - The identifier for the attribute that is intended to be changed.  
        - `value` (_str_) - The new value for said attribute.  
    - **Getter:** _[carla.ActorBlueprint.get_attribute](#carla.ActorBlueprint.get_attribute)_  

##### Dunder methods
- <a name="carla.ActorBlueprint.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.ActorAttribute](#carla.ActorAttribute) that this blueprint has.  
- <a name="carla.ActorBlueprint.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
Returns the amount of attributes for this blueprint.  
- <a name="carla.ActorBlueprint.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorList<a name="carla.ActorList"></a>
A class that contains every actor present on the scene and provides access to them. The list is automatically created and updated by the server and it can be returned using [carla.World](#carla.World).  

### Methods
- <a name="carla.ActorList.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Filters a list of Actors matching `wildcard_pattern` against their variable __<font color="#f8805a">type_id</font>__ (which identifies the blueprint used to spawn them). Matching follows [fnmatch](https://docs.python.org/2/library/fnmatch.html) standard.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _list_  
- <a name="carla.ActorList.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Finds an actor using its identifier and returns it or <b>None</b> if it is not present.  
    - **Parameters:**
        - `actor_id` (_int_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  

##### Dunder methods
- <a name="carla.ActorList.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
Returns the actor corresponding to `pos` position in the list.  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.ActorList.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.Actor](#carla.Actor) contained in the list.  
- <a name="carla.ActorList.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
Returns the amount of actors listed.  
    - **Return:** _int_  
- <a name="carla.ActorList.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses to the ID for every actor listed.  
    - **Return:** _str_  

---

## carla.ActorSnapshot<a name="carla.ActorSnapshot"></a>
A class that comprises all the information for an actor at a certain moment in time. These objects are contained in a [carla.WorldSnapshot](#carla.WorldSnapshot) and sent to the client once every tick.  

### Instance Variables
- <a name="carla.ActorSnapshot.id"></a>**<font color="#f8805a">id</font>** (_int_)  
An identifier for the snapshot itself.  

### Methods

##### Getters
- <a name="carla.ActorSnapshot.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
Returns the acceleration vector registered for an actor in that tick.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)<small> - m/s<sup>2</sup></small>_  
- <a name="carla.ActorSnapshot.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the angular velocity vector registered for an actor in that tick.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)<small> - rad/s</small>_  
- <a name="carla.ActorSnapshot.get_transform"></a>**<font color="#7fb800">get_transform</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's transform (location and rotation) for an actor in that tick.  
    - **Return:** _[carla.Transform](#carla.Transform)_  
- <a name="carla.ActorSnapshot.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the velocity vector registered for an actor in that tick.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)<small> - m/s</small>_  

---

## carla.ActorState<a name="carla.ActorState"></a>
Class that defines the state of an actor.  

### Instance Variables
- <a name="carla.ActorState.Invalid"></a>**<font color="#f8805a">Invalid</font>**  
An actor is Invalid if a problem has occurred.  
- <a name="carla.ActorState.Active"></a>**<font color="#f8805a">Active</font>**  
An actor is Active when it visualized and can affect other actors.  
- <a name="carla.ActorState.Dormant"></a>**<font color="#f8805a">Dormant</font>**  
An actor is Dormant when it is not visualized and will not affect other actors through physics. For example, actors are dormant if they are on an unloaded tile in a large map.  

---

## carla.AttachmentType<a name="carla.AttachmentType"></a>
Class that defines attachment options between an actor and its parent. When spawning actors, these can be attached to another actor so their position changes accordingly. This is specially useful for sensors. The snipet in [carla.World.spawn_actor](#carla.World.spawn_actor) shows some sensors being attached to a car when spawned. Note that the attachment type is declared as an enum within the class.  

### Instance Variables
- <a name="carla.AttachmentType.Rigid"></a>**<font color="#f8805a">Rigid</font>**  
With this fixed attachment the object follow its parent position strictly. This is the recommended attachment to retrieve precise data from the simulation.  
- <a name="carla.AttachmentType.SpringArm"></a>**<font color="#f8805a">SpringArm</font>**  
An attachment that expands or retracts the position of the actor, depending on its parent. This attachment is only recommended to record videos from the simulation where a smooth movement is needed. SpringArms are an Unreal Engine component so [check the UE docs](https://docs.unrealengine.com/en-US/Gameplay/HowTo/UsingCameras/SpringArmComponents/index.html) to learn more about them. <br><b style="color:red;">Warning:</b> The <b>SpringArm</b> attachment presents weird behaviors when an actor is spawned with a relative translation in the Z-axis (e.g. <code>child_location = Location(0,0,2)</code>).  
- <a name="carla.AttachmentType.SpringArmGhost"></a>**<font color="#f8805a">SpringArmGhost</font>**  
An attachment like the previous one but that does not make the collision test, and that means that it does not expands or retracts the position of the actor. The term **ghost** is because then the camera can cross walls and other geometries. This attachment is only recommended to record videos from the simulation where a smooth movement is needed. SpringArms are an Unreal Engine component so [check the UE docs](https://docs.unrealengine.com/en-US/Gameplay/HowTo/UsingCameras/SpringArmComponents/index.html) to learn more about them. <br><b style="color:red;">Warning:</b> The <b>SpringArm</b> attachment presents weird behaviors when an actor is spawned with a relative translation in the Z-axis (e.g. <code>child_location = Location(0,0,2)</code>).  

---

## carla.BlueprintLibrary<a name="carla.BlueprintLibrary"></a>
A class that contains the blueprints provided for actor spawning. Its main application is to return [carla.ActorBlueprint](#carla.ActorBlueprint) objects needed to spawn actors. Each blueprint has an identifier and attributes that may or may not be modifiable. The library is automatically created by the server and can be accessed through [carla.World](#carla.World).

  [Here](bp_library.md) is a reference containing every available blueprint and its specifics.  

### Methods
- <a name="carla.BlueprintLibrary.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Filters a list of blueprints matching the `wildcard_pattern` against the id and tags of every blueprint contained in this library and returns the result as a new one. Matching follows [fnmatch](https://docs.python.org/2/library/fnmatch.html) standard.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
- <a name="carla.BlueprintLibrary.filter_by_attribute"></a>**<font color="#7fb800">filter_by_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**value**</font>)  
Filters a list of blueprints with a given attribute matching the `value` against every blueprint contained in this library and returns the result as a new one. Matching follows [fnmatch](https://docs.python.org/2/library/fnmatch.html) standard.  
    - **Parameters:**
        - `name` (_str_)  
        - `value` (_str_)  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
- <a name="carla.BlueprintLibrary.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
Returns the blueprint corresponding to that identifier.  
    - **Parameters:**
        - `id` (_str_)  
    - **Return:** _[carla.ActorBlueprint](#carla.ActorBlueprint)_  

##### Dunder methods
- <a name="carla.BlueprintLibrary.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
Returns the blueprint stored in `pos` position inside the data structure containing them.  
    - **Return:** _[carla.ActorBlueprint](#carla.ActorBlueprint)_  
- <a name="carla.BlueprintLibrary.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.ActorBlueprint](#carla.ActorBlueprint) stored in the library.  
- <a name="carla.BlueprintLibrary.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
Returns the amount of blueprints comprising the library.  
    - **Return:** _int_  
- <a name="carla.BlueprintLibrary.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses the identifiers for every blueprint to string.  
    - **Return:** _string_  

---

## carla.BoundingBox<a name="carla.BoundingBox"></a>
Bounding boxes contain the geometry of an actor or an element in the scene. They can be used by [carla.DebugHelper](#carla.DebugHelper) or a [carla.Client](#carla.Client) to draw their shapes for debugging. Check out the snipet in [carla.DebugHelper.draw_box](#carla.DebugHelper.draw_box) where a snapshot of the world is used to draw bounding boxes for traffic lights.  

### Instance Variables
- <a name="carla.BoundingBox.extent"></a>**<font color="#f8805a">extent</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - meters</small>_)  
Vector from the center of the box to one vertex. The value in each axis equals half the size of the box for that axis.
`extent.x * 2` would return the size of the box in the X-axis.  
- <a name="carla.BoundingBox.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)<small> - meters</small>_)  
The center of the bounding box.  
- <a name="carla.BoundingBox.rotation"></a>**<font color="#f8805a">rotation</font>** (_[carla.Rotation](#carla.Rotation)_)  
The orientation of the bounding box.  

### Methods
- <a name="carla.BoundingBox.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**extent**</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) - Center of the box, relative to its parent.  
        - `extent` (_[carla.Vector3D](#carla.Vector3D)<small> - meters</small>_) - Vector containing half the size of the box for every axis.  
- <a name="carla.BoundingBox.contains"></a>**<font color="#7fb800">contains</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**world_point**</font>, <font color="#00a6ed">**transform**</font>)  
Returns **True** if a point passed in world space is inside this bounding box.  
    - **Parameters:**
        - `world_point` (_[carla.Location](#carla.Location)<small> - meters</small>_) - The point in world space to be checked.  
        - `transform` (_[carla.Transform](#carla.Transform)_) - Contains location and rotation needed to convert this object's local space to world space.  
    - **Return:** _bool_  

##### Getters
- <a name="carla.BoundingBox.get_local_vertices"></a>**<font color="#7fb800">get_local_vertices</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list containing the locations of this object's vertices in local space.  
    - **Return:** _list([carla.Location](#carla.Location))_  
- <a name="carla.BoundingBox.get_world_vertices"></a>**<font color="#7fb800">get_world_vertices</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**transform**</font>)  
Returns a list containing the locations of this object's vertices in world space.  
    - **Parameters:**
        - `transform` (_[carla.Transform](#carla.Transform)_) - Contains location and rotation needed to convert this object's local space to world space.  
    - **Return:** _list([carla.Location](#carla.Location))_  

##### Dunder methods
- <a name="carla.BoundingBox.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.BoundingBox](#carla.BoundingBox)</font>)  
Returns true if both location and extent are equal for this and `other`.  
    - **Return:** _bool_  
- <a name="carla.BoundingBox.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.BoundingBox](#carla.BoundingBox)</font>)  
Returns true if either location or extent are different for this and `other`.  
    - **Return:** _bool_  
- <a name="carla.BoundingBox.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses the location and extent of the bounding box to string.  
    - **Return:** _str_  

---

## carla.CityObjectLabel<a name="carla.CityObjectLabel"></a>
Enum declaration that contains the different tags available to filter the bounding boxes returned by [carla.World.get_level_bbs](#carla.World.get_level_bbs)(). These values correspond to the [semantic tag](ref_sensors.md#semantic-segmentation-camera) that the elements in the scene have.  

### Instance Variables
- <a name="carla.CityObjectLabel.None"></a>**<font color="#f8805a">None</font>**  
- <a name="carla.CityObjectLabel.Buildings"></a>**<font color="#f8805a">Buildings</font>**  
- <a name="carla.CityObjectLabel.Fences"></a>**<font color="#f8805a">Fences</font>**  
- <a name="carla.CityObjectLabel.Other"></a>**<font color="#f8805a">Other</font>**  
- <a name="carla.CityObjectLabel.Pedestrians"></a>**<font color="#f8805a">Pedestrians</font>**  
- <a name="carla.CityObjectLabel.Poles"></a>**<font color="#f8805a">Poles</font>**  
- <a name="carla.CityObjectLabel.RoadLines"></a>**<font color="#f8805a">RoadLines</font>**  
- <a name="carla.CityObjectLabel.Roads"></a>**<font color="#f8805a">Roads</font>**  
- <a name="carla.CityObjectLabel.Sidewalks"></a>**<font color="#f8805a">Sidewalks</font>**  
- <a name="carla.CityObjectLabel.TrafficSigns"></a>**<font color="#f8805a">TrafficSigns</font>**  
- <a name="carla.CityObjectLabel.Vegetation"></a>**<font color="#f8805a">Vegetation</font>**  
- <a name="carla.CityObjectLabel.Vehicles"></a>**<font color="#f8805a">Vehicles</font>**  
- <a name="carla.CityObjectLabel.Walls"></a>**<font color="#f8805a">Walls</font>**  
- <a name="carla.CityObjectLabel.Sky"></a>**<font color="#f8805a">Sky</font>**  
- <a name="carla.CityObjectLabel.Ground"></a>**<font color="#f8805a">Ground</font>**  
- <a name="carla.CityObjectLabel.Bridge"></a>**<font color="#f8805a">Bridge</font>**  
- <a name="carla.CityObjectLabel.RailTrack"></a>**<font color="#f8805a">RailTrack</font>**  
- <a name="carla.CityObjectLabel.GuardRail"></a>**<font color="#f8805a">GuardRail</font>**  
- <a name="carla.CityObjectLabel.TrafficLight"></a>**<font color="#f8805a">TrafficLight</font>**  
- <a name="carla.CityObjectLabel.Static"></a>**<font color="#f8805a">Static</font>**  
- <a name="carla.CityObjectLabel.Dynamic"></a>**<font color="#f8805a">Dynamic</font>**  
- <a name="carla.CityObjectLabel.Water"></a>**<font color="#f8805a">Water</font>**  
- <a name="carla.CityObjectLabel.Terrain"></a>**<font color="#f8805a">Terrain</font>**  
- <a name="carla.CityObjectLabel.Any"></a>**<font color="#f8805a">Any</font>**  

---

## carla.Client<a name="carla.Client"></a>
The Client connects CARLA to the server which runs the simulation. Both server and client contain a CARLA library (libcarla) with some differences that allow communication between them. Many clients can be created and each of these will connect to the RPC server inside the simulation to send commands. The simulation runs server-side. Once the connection is established, the client will only receive data retrieved from the simulation. Walkers are the exception. The client is in charge of managing pedestrians so, if you are running a simulation with multiple clients, some issues may arise. For example, if you spawn walkers through different clients, collisions may happen, as each client is only aware of the ones it is in charge of.

  The client also has a recording feature that saves all the information of a simulation while running it. This allows the server to replay it at will to obtain information and experiment with it. [Here](adv_recorder.md) is some information about how to use this recorder.  

### Methods
- <a name="carla.Client.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**host**=127.0.0.1</font>, <font color="#00a6ed">**port**=2000</font>, <font color="#00a6ed">**worker_threads**=0</font>)<button class="SnipetButton" id="carla.Client.__init__-snipet_button">snippet &rarr;</button>  
Client constructor.  
    - **Parameters:**
        - `host` (_str_) - IP address where a CARLA Simulator instance is running. Default is localhost (127.0.0.1).  
        - `port` (_int_) - TCP port where the CARLA Simulator instance is running. Default are 2000 and the subsequent 2001.  
        - `worker_threads` (_int_) - Number of working threads used for background updates. If 0, use all available concurrency.  
- <a name="carla.Client.apply_batch"></a>**<font color="#7fb800">apply_batch</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**commands**</font>)  
Executes a list of commands on a single simulation step and retrieves no information. If you need information about the response of each command, use the __<font color="#7fb800">apply_batch_sync()</font>__ method. [Here](https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/generate_traffic.py) is an example on how to delete the actors that appear in [carla.ActorList](#carla.ActorList) all at once.  
    - **Parameters:**
        - `commands` (_list_) - A list of commands to execute in batch. Each command is different and has its own parameters. They appear listed at the bottom of this page.  
- <a name="carla.Client.apply_batch_sync"></a>**<font color="#7fb800">apply_batch_sync</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**commands**</font>, <font color="#00a6ed">**due_tick_cue**=False</font>)<button class="SnipetButton" id="carla.Client.apply_batch_sync-snipet_button">snippet &rarr;</button>  
Executes a list of commands on a single simulation step, blocks until the commands are linked, and returns a list of <b>command.Response</b> that can be used to determine whether a single command succeeded or not. [Here](https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/generate_traffic.py) is an example of it being used to spawn actors.  
    - **Parameters:**
        - `commands` (_list_) - A list of commands to execute in batch. The commands available are listed right above, in the method **<font color="#7fb800">apply_batch()</font>**.  
        - `due_tick_cue` (_bool_) - A boolean parameter to specify whether or not to perform a [carla.World.tick](#carla.World.tick) after applying the batch in _synchronous mode_. It is __False__ by default.  
    - **Return:** _list(command.Response)_  
- <a name="carla.Client.generate_opendrive_world"></a>**<font color="#7fb800">generate_opendrive_world</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**opendrive**</font>, <font color="#00a6ed">**parameters**=(2.0, 50.0, 1.0, 0.6, true, true)</font>, <font color="#00a6ed">**reset_settings**=True</font>)  
Loads a new world with a basic 3D topology generated from the content of an OpenDRIVE file. This content is passed as a `string` parameter. It is similar to `client.load_world(map_name)` but allows for custom OpenDRIVE maps in server side. Cars can drive around the map, but there are no graphics besides the road and sidewalks.  
    - **Parameters:**
        - `opendrive` (_str_) - Content of an OpenDRIVE file as `string`, __not the path to the `.xodr`__.  
        - `parameters` (_[carla.OpendriveGenerationParameters](#carla.OpendriveGenerationParameters)_) - Additional settings for the mesh generation. If none are provided, default values will be used.  
        - `reset_settings` (_bool_) - Option to reset the episode setting to default values, set to false to keep the current settings. This is useful to keep sync mode when changing map and to keep deterministic scenarios.  
- <a name="carla.Client.load_world"></a>**<font color="#7fb800">load_world</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**map_name**</font>, <font color="#00a6ed">**reset_settings**=True</font>, <font color="#00a6ed">**map_layers**=[carla.MapLayer.All](#carla.MapLayer.All)</font>)  
Creates a new world with default settings using `map_name` map. All actors in the current world will be destroyed.  
    - **Parameters:**
        - `map_name` (_str_) - Name of the map to be used in this world. Accepts both full paths and map names, e.g. '/Game/Carla/Maps/Town01' or 'Town01'. Remember that these paths are dynamic.  
        - `reset_settings` (_bool_) - Option to reset the episode setting to default values, set to false to keep the current settings. This is useful to keep sync mode when changing map and to keep deterministic scenarios.  
        - `map_layers` (_[carla.MapLayer](#carla.MapLayer)_) - Layers of the map that will be loaded. By default all layers are loaded. This parameter works like a flag mask.  
    - **Warning:** <font color="#ED2F2F">_`map_layers` are only available for "Opt" maps
_</font>  
- <a name="carla.Client.reload_world"></a>**<font color="#7fb800">reload_world</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**reset_settings**=True</font>)  
Reload the current world, note that a new world is created with default settings using the same map. All actors present in the world will be destroyed, __but__ traffic manager instances will stay alive.  
    - **Parameters:**
        - `reset_settings` (_bool_) - Option to reset the episode setting to default values, set to false to keep the current settings. This is useful to keep sync mode when changing map and to keep deterministic scenarios.  
    - **Raises:** RuntimeError when corresponding.  
- <a name="carla.Client.replay_file"></a>**<font color="#7fb800">replay_file</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**start**</font>, <font color="#00a6ed">**duration**</font>, <font color="#00a6ed">**follow_id**</font>, <font color="#00a6ed">**replay_sensors**</font>)  
Load a new world with default settings using `map_name` map. All actors present in the current world will be destroyed, __but__ traffic manager instances will stay alive.  
    - **Parameters:**
        - `name` (_str_) - Name of the file containing the information of the simulation.  
        - `start` (_float<small> - seconds</small>_) - Time where to start playing the simulation. Negative is read as beginning from the end, being -10 just 10 seconds before the recording finished.  
        - `duration` (_float<small> - seconds</small>_) - Time that will be reenacted using the information `name` file. If the end is reached, the simulation will continue.  
        - `follow_id` (_int_) - ID of the actor to follow. If this is 0 then camera is disabled.  
        - `replay_sensors` (_bool_) - Flag to enable or disable the spawn of sensors during playback.  
- <a name="carla.Client.request_file"></a>**<font color="#7fb800">request_file</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>)  
Requests one of the required files returned by [carla.Client.get_required_files](#carla.Client.get_required_files).  
    - **Parameters:**
        - `name` (_str_) - Name of the file you are requesting.  
- <a name="carla.Client.show_recorder_actors_blocked"></a>**<font color="#7fb800">show_recorder_actors_blocked</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**min_time**</font>, <font color="#00a6ed">**min_distance**</font>)  
The terminal will show the information registered for actors considered blocked. An actor is considered blocked when it does not move a minimum distance in a period of time, being these `min_distance` and `min_time`.  
    - **Parameters:**
        - `filename` (_str_) - Name of the recorded file to load.  
        - `min_time` (_float<small> - seconds</small>_) - Minimum time the actor has to move a minimum distance before being considered blocked. Default is 60 seconds.  
        - `min_distance` (_float<small> - centimeters</small>_) - Minimum distance the actor has to move to not be considered blocked. Default is 100 centimeters.  
    - **Return:** _string_  
- <a name="carla.Client.show_recorder_collisions"></a>**<font color="#7fb800">show_recorder_collisions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**category1**</font>, <font color="#00a6ed">**category2**</font>)  
The terminal will show the collisions registered by the recorder. These can be filtered by specifying the type of actor involved. The categories will be specified in `category1` and `category2` as follows:
  'h' = Hero, the one vehicle that can be controlled manually or managed by the user.
  'v' = Vehicle
  'w' = Walker
  't' = Traffic light
  'o' = Other
  'a' = Any
If you want to see only collisions between a vehicles and a walkers, use for `category1` as 'v' and `category2` as 'w' or vice versa. If you want to see all the collisions (filter off) you can use 'a' for both parameters.  
    - **Parameters:**
        - `filename` (_str_) - Name or absolute path of the file recorded, depending on your previous choice.  
        - `category1` (_single char_) - Character variable specifying a first type of actor involved in the collision.  
        - `category2` (_single char_) - Character variable specifying the second type of actor involved in the collision.  
    - **Return:** _string_  
- <a name="carla.Client.show_recorder_file_info"></a>**<font color="#7fb800">show_recorder_file_info</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**show_all**</font>)  
The information saved by the recorder will be parsed and shown in your terminal as text (frames, times, events, state, positions...). The information shown can be specified by using the `show_all` parameter. [Here](ref_recorder_binary_file_format.md) is some more information about how to read the recorder file.  
    - **Parameters:**
        - `filename` (_str_) - Name or absolute path of the file recorded, depending on your previous choice.  
        - `show_all` (_bool_) - If __True__, returns all the information stored for every frame (traffic light states, positions of all actors, orientation and animation data...). If __False__, returns a summary of key events and frames.  
    - **Return:** _string_  
- <a name="carla.Client.start_recorder"></a>**<font color="#7fb800">start_recorder</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**additional_data**=False</font>)  
Enables the recording feature, which will start saving every information possible needed by the server to replay the simulation.  
    - **Parameters:**
        - `filename` (_str_) - Name of the file to write the recorded data. A simple name will save the recording in 'CarlaUE4/Saved/recording.log'. Otherwise, if some folder appears in the name, it will be considered an absolute path.  
        - `additional_data` (_bool_) - Enables or disable recording non-essential data for reproducing the simulation (bounding box location, physics control parameters, etc).  
- <a name="carla.Client.stop_recorder"></a>**<font color="#7fb800">stop_recorder</font>**(<font color="#00a6ed">**self**</font>)  
Stops the recording in progress. If you specified a path in `filename`, the recording will be there. If not, look inside `CarlaUE4/Saved/`.  
- <a name="carla.Client.stop_replayer"></a>**<font color="#7fb800">stop_replayer</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**keep_actors**</font>)  
Stop current replayer.  
    - **Parameters:**
        - `keep_actors` (_bool_) - True if you want autoremove all actors from the replayer, or False to keep them.  

##### Getters
- <a name="carla.Client.get_available_maps"></a>**<font color="#7fb800">get_available_maps</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of strings containing the paths of the maps available on server. These paths are dynamic, they will be created during the simulation and so you will not find them when looking up in your files. One of the possible returns for this method would be:
  ['/Game/Carla/Maps/Town01',
  '/Game/Carla/Maps/Town02',
  '/Game/Carla/Maps/Town03',
  '/Game/Carla/Maps/Town04',
  '/Game/Carla/Maps/Town05',
  '/Game/Carla/Maps/Town06',
  '/Game/Carla/Maps/Town07'].  
    - **Return:** _list(str)_  
- <a name="carla.Client.get_client_version"></a>**<font color="#7fb800">get_client_version</font>**(<font color="#00a6ed">**self**</font>)  
Returns the client libcarla version by consulting it in the "Version.h" file. Both client and server can use different libcarla versions but some issues may arise regarding unexpected incompatibilities.  
    - **Return:** _str_  
- <a name="carla.Client.get_required_files"></a>**<font color="#7fb800">get_required_files</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**folder**</font>, <font color="#00a6ed">**download**=True</font>)  
Asks the server which files are required by the client to use the current map. Option to download files automatically if they are not already in the cache.  
    - **Parameters:**
        - `folder` (_str_) - Folder where files required by the client will be downloaded to.  
        - `download` (_bool_) - If True, downloads files that are not already in cache.  
- <a name="carla.Client.get_server_version"></a>**<font color="#7fb800">get_server_version</font>**(<font color="#00a6ed">**self**</font>)  
Returns the server libcarla version by consulting it in the "Version.h" file. Both client and server should use the same libcarla version.  
    - **Return:** _str_  
- <a name="carla.Client.get_trafficmanager"></a>**<font color="#7fb800">get_trafficmanager</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**client_connection**=8000</font>)  
Returns an instance of the traffic manager related to the specified port. If it does not exist, this will be created.  
    - **Parameters:**
        - `client_connection` (_int_) - Port that will be used by the traffic manager. Default is `8000`.  
    - **Return:** _[carla.TrafficManager](#carla.TrafficManager)_  
- <a name="carla.Client.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Returns the world object currently active in the simulation. This world will be later used for example to load maps.  
    - **Return:** _[carla.World](#carla.World)_  

##### Setters
- <a name="carla.Client.set_files_base_folder"></a>**<font color="#7fb800">set_files_base_folder</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
    - **Parameters:**
        - `path` (_str_) - Specifies the base folder where the local cache for required files will be placed.  
- <a name="carla.Client.set_replayer_ignore_hero"></a>**<font color="#7fb800">set_replayer_ignore_hero</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**ignore_hero**</font>)  
    - **Parameters:**
        - `ignore_hero` (_bool_) - Enables or disables playback of the hero vehicle during a playback of a recorded simulation.  
- <a name="carla.Client.set_replayer_ignore_spectator"></a>**<font color="#7fb800">set_replayer_ignore_spectator</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**ignore_spectator**</font>)  
    - **Parameters:**
        - `ignore_spectator` (_bool_) - Determines whether the recorded spectator movements will be replicated by the replayer.  
- <a name="carla.Client.set_replayer_time_factor"></a>**<font color="#7fb800">set_replayer_time_factor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**time_factor**=1.0</font>)  
When used, the time speed of the reenacted simulation is modified at will. It can be used several times while a playback is in curse.  
    - **Parameters:**
        - `time_factor` (_float_) - 1.0 means normal time speed. Greater than 1.0 means fast motion (2.0 would be double speed) and lesser means slow motion (0.5 would be half speed).  
- <a name="carla.Client.set_timeout"></a>**<font color="#7fb800">set_timeout</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**</font>)  
Sets the maximum time a network call is allowed before blocking it and raising a timeout exceeded error.  
    - **Parameters:**
        - `seconds` (_float<small> - seconds</small>_) - New timeout value. Default is 5 seconds.  

---

## carla.CollisionEvent<a name="carla.CollisionEvent"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines a collision data for <b>sensor.other.collision</b>. The sensor creates one of these for every collision detected. Each collision sensor produces one collision event per collision per frame. Multiple collision events may be produced in a single frame by collisions with multiple other actors. Learn more about this [here](ref_sensors.md#collision-detector).  

### Instance Variables
- <a name="carla.CollisionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
The actor the sensor is attached to, the one that measured the collision.  
- <a name="carla.CollisionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
The second actor involved in the collision.  
- <a name="carla.CollisionEvent.normal_impulse"></a>**<font color="#f8805a">normal_impulse</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - N*s</small>_)  
Normal impulse resulting of the collision.  

---

## carla.Color<a name="carla.Color"></a>
Class that defines a 32-bit RGBA color.  

### Instance Variables
- <a name="carla.Color.r"></a>**<font color="#f8805a">r</font>** (_int_)  
Red color (0-255).  
- <a name="carla.Color.g"></a>**<font color="#f8805a">g</font>** (_int_)  
Green color (0-255).  
- <a name="carla.Color.b"></a>**<font color="#f8805a">b</font>** (_int_)  
Blue color (0-255).  
- <a name="carla.Color.a"></a>**<font color="#f8805a">a</font>** (_int_)  
Alpha channel (0-255).  

### Methods
- <a name="carla.Color.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**r**=0</font>, <font color="#00a6ed">**g**=0</font>, <font color="#00a6ed">**b**=0</font>, <font color="#00a6ed">**a**=255</font>)  
Initializes a color, black by default.  
    - **Parameters:**
        - `r` (_int_)  
        - `g` (_int_)  
        - `b` (_int_)  
        - `a` (_int_)  

##### Dunder methods
- <a name="carla.Color.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Color](#carla.Color)</font>)  
- <a name="carla.Color.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Color](#carla.Color)</font>)  
- <a name="carla.Color.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ColorConverter<a name="carla.ColorConverter"></a>
Class that defines conversion patterns that can be applied to a [carla.Image](#carla.Image) in order to show information provided by [carla.Sensor](#carla.Sensor). Depth conversions cause a loss of accuracy, as sensors detect depth as <b>float</b> that is then converted to a grayscale value between 0 and 255. Take a look at the snipet in [carla.Sensor.listen](#carla.Sensor.listen) to see an example of how to create and save image data for <b>sensor.camera.semantic_segmentation</b>.  

### Instance Variables
- <a name="carla.ColorConverter.CityScapesPalette"></a>**<font color="#f8805a">CityScapesPalette</font>**  
Converts the image to a segmented map using tags provided by the blueprint library. Used by the [semantic segmentation camera](ref_sensors.md#semantic-segmentation-camera).  
- <a name="carla.ColorConverter.Depth"></a>**<font color="#f8805a">Depth</font>**  
Converts the image to a linear depth map. Used by the [depth camera](ref_sensors.md#depth-camera).  
- <a name="carla.ColorConverter.LogarithmicDepth"></a>**<font color="#f8805a">LogarithmicDepth</font>**  
Converts the image to a depth map using a logarithmic scale, leading to better precision for small distances at the expense of losing it when further away.  
- <a name="carla.ColorConverter.Raw"></a>**<font color="#f8805a">Raw</font>**  
No changes applied to the image. Used by the [RGB camera](ref_sensors.md#rgb-camera).  

---

## carla.DVSEvent<a name="carla.DVSEvent"></a>
Class that defines a DVS event. An event is a quadruple, so a tuple of 4 elements, with `x`, `y` pixel coordinate location, timestamp `t` and polarity `pol` of the event. Learn more about them [here](ref_sensors.md).  

### Instance Variables
- <a name="carla.DVSEvent.x"></a>**<font color="#f8805a">x</font>** (_int_)  
X pixel coordinate.  
- <a name="carla.DVSEvent.y"></a>**<font color="#f8805a">y</font>** (_int_)  
Y pixel coordinate.  
- <a name="carla.DVSEvent.t"></a>**<font color="#f8805a">t</font>** (_int_)  
Timestamp of the moment the event happened.  
- <a name="carla.DVSEvent.pol"></a>**<font color="#f8805a">pol</font>** (_bool_)  
Polarity of the event. __True__ for positive and __False__ for negative.  

### Methods

##### Dunder methods
- <a name="carla.DVSEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.DVSEventArray<a name="carla.DVSEventArray"></a>
Class that defines a stream of events in [carla.DVSEvent](#carla.DVSEvent). Such stream is an array of arbitrary size depending on the number of events. This class also stores the field of view, the height and width of the image and the timestamp from convenience. Learn more about them [here](ref_sensors.md).  

### Instance Variables
- <a name="carla.DVSEventArray.fov"></a>**<font color="#f8805a">fov</font>** (_float<small> - degrees</small>_)  
Horizontal field of view of the image.  
- <a name="carla.DVSEventArray.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Image height in pixels.  
- <a name="carla.DVSEventArray.width"></a>**<font color="#f8805a">width</font>** (_int_)  
Image width in pixels.  
- <a name="carla.DVSEventArray.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  

### Methods
- <a name="carla.DVSEventArray.to_array"></a>**<font color="#7fb800">to_array</font>**(<font color="#00a6ed">**self**</font>)  
Converts the stream of events to an array of int values in the following order <code>[x, y, t, pol]</code>.  
- <a name="carla.DVSEventArray.to_array_pol"></a>**<font color="#7fb800">to_array_pol</font>**(<font color="#00a6ed">**self**</font>)  
Returns an array with the polarity of all the events in the stream.  
- <a name="carla.DVSEventArray.to_array_t"></a>**<font color="#7fb800">to_array_t</font>**(<font color="#00a6ed">**self**</font>)  
Returns an array with the timestamp of all the events in the stream.  
- <a name="carla.DVSEventArray.to_array_x"></a>**<font color="#7fb800">to_array_x</font>**(<font color="#00a6ed">**self**</font>)  
Returns an array with X pixel coordinate of all the events in the stream.  
- <a name="carla.DVSEventArray.to_array_y"></a>**<font color="#7fb800">to_array_y</font>**(<font color="#00a6ed">**self**</font>)  
Returns an array with Y pixel coordinate of all the events in the stream.  
- <a name="carla.DVSEventArray.to_image"></a>**<font color="#7fb800">to_image</font>**(<font color="#00a6ed">**self**</font>)  
Converts the image following this pattern: blue indicates positive events, red indicates negative events.  

##### Dunder methods
- <a name="carla.DVSEventArray.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
- <a name="carla.DVSEventArray.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.DVSEvent](#carla.DVSEvent) retrieved as data.  
- <a name="carla.DVSEventArray.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.DVSEventArray.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>, <font color="#00a6ed">**color**=[carla.Color](#carla.Color)</font>)  
- <a name="carla.DVSEventArray.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.DebugHelper<a name="carla.DebugHelper"></a>
Helper class part of [carla.World](#carla.World) that defines methods for creating debug shapes. By default, shapes last one second. They can be permanent, but take into account the resources needed to do so. Take a look at the snipets available for this class to learn how to debug easily in CARLA.  

### Methods
- <a name="carla.DebugHelper.draw_arrow"></a>**<font color="#7fb800">draw_arrow</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**begin**</font>, <font color="#00a6ed">**end**</font>, <font color="#00a6ed">**thickness**=0.1</font>, <font color="#00a6ed">**arrow_size**=0.1</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0</font>)  
Draws an arrow from `begin` to `end` pointing in that direction.  
    - **Parameters:**
        - `begin` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Point in the coordinate system where the arrow starts.  
        - `end` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Point in the coordinate system where the arrow ends and points towards to.  
        - `thickness` (_float<small> - meters</small>_) - Density of the line.  
        - `arrow_size` (_float<small> - meters</small>_) - Size of the tip of the arrow.  
        - `color` (_[carla.Color](#carla.Color)_) - RGB code to color the object. Red by default.  
        - `life_time` (_float<small> - seconds</small>_) - Shape's lifespan. By default it only lasts one frame. Set this to <code>0</code> for permanent shapes.  
- <a name="carla.DebugHelper.draw_box"></a>**<font color="#7fb800">draw_box</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**box**</font>, <font color="#00a6ed">**rotation**</font>, <font color="#00a6ed">**thickness**=0.1</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0</font>)<button class="SnipetButton" id="carla.DebugHelper.draw_box-snipet_button">snippet &rarr;</button>  
Draws a box, ussually to act for object colliders.  
    - **Parameters:**
        - `box` (_[carla.BoundingBox](#carla.BoundingBox)_) - Object containing a location and the length of a box for every axis.  
        - `rotation` (_[carla.Rotation](#carla.Rotation)<small> - degrees (pitch,yaw,roll)</small>_) - Orientation of the box according to Unreal Engine's axis system.  
        - `thickness` (_float<small> - meters</small>_) - Density of the lines that define the box.  
        - `color` (_[carla.Color](#carla.Color)_) - RGB code to color the object. Red by default.  
        - `life_time` (_float<small> - seconds</small>_) - Shape's lifespan. By default it only lasts one frame. Set this to <code>0</code> for permanent shapes.  
- <a name="carla.DebugHelper.draw_line"></a>**<font color="#7fb800">draw_line</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**begin**</font>, <font color="#00a6ed">**end**</font>, <font color="#00a6ed">**thickness**=0.1</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0</font>)  
Draws a line in between `begin` and `end`.  
    - **Parameters:**
        - `begin` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Point in the coordinate system where the line starts.  
        - `end` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Spot in the coordinate system where the line ends.  
        - `thickness` (_float<small> - meters</small>_) - Density of the line.  
        - `color` (_[carla.Color](#carla.Color)_) - RGB code to color the object. Red by default.  
        - `life_time` (_float<small> - seconds</small>_) - Shape's lifespan. By default it only lasts one frame. Set this to <code>0</code> for permanent shapes.  
- <a name="carla.DebugHelper.draw_point"></a>**<font color="#7fb800">draw_point</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**size**=0.1</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0</font>)  
Draws a point `location`.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Spot in the coordinate system to center the object.  
        - `size` (_float<small> - meters</small>_) - Density of the point.  
        - `color` (_[carla.Color](#carla.Color)_) - RGB code to color the object. Red by default.  
        - `life_time` (_float<small> - seconds</small>_) - Shape's lifespan. By default it only lasts one frame. Set this to <code>0</code> for permanent shapes.  
- <a name="carla.DebugHelper.draw_string"></a>**<font color="#7fb800">draw_string</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**text**</font>, <font color="#00a6ed">**draw_shadow**=False</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0</font>)<button class="SnipetButton" id="carla.DebugHelper.draw_string-snipet_button">snippet &rarr;</button>  
Draws a string in a given location of the simulation which can only be seen server-side.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Spot in the simulation where the text will be centered.  
        - `text` (_str_) - Text intended to be shown in the world.  
        - `draw_shadow` (_bool_) - Casts a shadow for the string that could help in visualization. It is disabled by default.  
        - `color` (_[carla.Color](#carla.Color)_) - RGB code to color the string. Red by default.  
        - `life_time` (_float<small> - seconds</small>_) - Shape's lifespan. By default it only lasts one frame. Set this to <code>0</code> for permanent shapes.  

---

## carla.EnvironmentObject<a name="carla.EnvironmentObject"></a>
Class that represents a geometry in the level, this geometry could be part of an actor formed with other EnvironmentObjects (ie: buildings).  

### Instance Variables
- <a name="carla.EnvironmentObject.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Contains the location and orientation of the EnvironmentObject in world space.  
- <a name="carla.EnvironmentObject.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
Object containing a location, rotation and the length of a box for every axis in world space.  
- <a name="carla.EnvironmentObject.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Unique ID to identify the object in the level.  
- <a name="carla.EnvironmentObject.name"></a>**<font color="#f8805a">name</font>** (_string_)  
Name of the EnvironmentObject.  
- <a name="carla.EnvironmentObject.type"></a>**<font color="#f8805a">type</font>** (_[carla.CityObjectLabel](#carla.CityObjectLabel)_)  
Semantic tag.  

### Methods

##### Dunder methods
- <a name="carla.EnvironmentObject.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses the EnvironmentObject to a string and shows them in command line.  
    - **Return:** _str_  

---

## carla.FloatColor<a name="carla.FloatColor"></a>
Class that defines a float RGBA color.  

### Instance Variables
- <a name="carla.FloatColor.r"></a>**<font color="#f8805a">r</font>** (_float_)  
Red color.  
- <a name="carla.FloatColor.g"></a>**<font color="#f8805a">g</font>** (_float_)  
Green color.  
- <a name="carla.FloatColor.b"></a>**<font color="#f8805a">b</font>** (_float_)  
Blue color.  
- <a name="carla.FloatColor.a"></a>**<font color="#f8805a">a</font>** (_float_)  
Alpha channel.  

### Methods
- <a name="carla.FloatColor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**r**=0</font>, <font color="#00a6ed">**g**=0</font>, <font color="#00a6ed">**b**=0</font>, <font color="#00a6ed">**a**=1.0</font>)  
Initializes a color, black by default.  
    - **Parameters:**
        - `r` (_float_)  
        - `g` (_float_)  
        - `b` (_float_)  
        - `a` (_float_)  

##### Dunder methods
- <a name="carla.FloatColor.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.FloatColor](#carla.FloatColor)</font>)  
- <a name="carla.FloatColor.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.FloatColor](#carla.FloatColor)</font>)  

---

## carla.GBufferTextureID<a name="carla.GBufferTextureID"></a>
Defines the identifiers of each GBuffer texture (See the method `[carla.Sensor.listen_to_gbuffer](#carla.Sensor.listen_to_gbuffer)`).  

### Instance Variables
- <a name="carla.GBufferTextureID.SceneColor"></a>**<font color="#f8805a">SceneColor</font>**  
The texture "SceneColor" contains the final color of the image.  
- <a name="carla.GBufferTextureID.SceneDepth"></a>**<font color="#f8805a">SceneDepth</font>**  
The texture "SceneDepth" contains the depth buffer - linear in world units.  
- <a name="carla.GBufferTextureID.SceneStencil"></a>**<font color="#f8805a">SceneStencil</font>**  
The texture "SceneStencil" contains the stencil buffer.  
- <a name="carla.GBufferTextureID.GBufferA"></a>**<font color="#f8805a">GBufferA</font>**  
The texture "GBufferA" contains the world-space normal vectors in the RGB channels. The alpha channel contains "per-object data".  
- <a name="carla.GBufferTextureID.GBufferB"></a>**<font color="#f8805a">GBufferB</font>**  
The texture "GBufferB" contains the metallic, specular and roughness in the RGB channels, respectively. The alpha channel contains a mask where the lower 4 bits indicate the shading model and the upper 4 bits contain the selective output mask.  
- <a name="carla.GBufferTextureID.GBufferC"></a>**<font color="#f8805a">GBufferC</font>**  
The texture "GBufferC" contains the diffuse color in the RGB channels, with the indirect irradiance in the alpha channel.<br> If static lightning is not allowed, the alpha channel will contain the ambient occlusion instead.  
- <a name="carla.GBufferTextureID.GBufferD"></a>**<font color="#f8805a">GBufferD</font>**  
The contents of the "GBufferD" varies depending on the rendered object's material shading model (GBufferB):<br>
  - MSM_Subsurface (2), MSM_PreintegratedSkin (3), MSM_TwoSidedFoliage (6):<br>
    RGB: Subsurface color.<br>
    A: Opacity.<br>
  - MSM_ClearCoat (4):<br>
    R: Clear coat.<br>
    G: Roughness.<br>
  - MSM_SubsurfaceProfile (5):<br>
    RGB: Subsurface profile.<br>
  - MSM_Hair (7):<br>
    RG: World normal.<br>
    B: Backlit value.<br>
  - MSM_Cloth (8):<br>
    RGB: Subsurface color.<br>
    A: Cloth value.<br>
  - MSM_Eye (9):<br>
    RG: Eye tangent.<br>
    B: Iris mask.<br>
    A: Iris distance.  
- <a name="carla.GBufferTextureID.GBufferE"></a>**<font color="#f8805a">GBufferE</font>**  
The texture "GBufferE" contains the precomputed shadow factors in the RGBA channels. This texture is unavailable if the selective output mask (GBufferB) does not have its 4th bit set.  
- <a name="carla.GBufferTextureID.GBufferF"></a>**<font color="#f8805a">GBufferF</font>**  
The texture "GBufferF" contains the world-space tangent in the RGB channels and the anisotropy in the alpha channel. This texture is unavailable if the selective output mask (GBufferB) does not have its 5th bit set.  
- <a name="carla.GBufferTextureID.Velocity"></a>**<font color="#f8805a">Velocity</font>**  
The texture "Velocity" contains the screen-space velocity of the scene objects.  
- <a name="carla.GBufferTextureID.SSAO"></a>**<font color="#f8805a">SSAO</font>**  
The texture "SSAO" contains the screen-space ambient occlusion texture.  
- <a name="carla.GBufferTextureID.CustomDepth"></a>**<font color="#f8805a">CustomDepth</font>**  
The texture "CustomDepth" contains the Unreal Engine custom depth data.  
- <a name="carla.GBufferTextureID.CustomStencil"></a>**<font color="#f8805a">CustomStencil</font>**  
The texture "CustomStencil" contains the Unreal Engine custom stencil data.  

---

## carla.GearPhysicsControl<a name="carla.GearPhysicsControl"></a>
Class that provides access to vehicle transmission details by defining a gear and when to run on it. This will be later used by [carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl) to help simulate physics.  

### Instance Variables
- <a name="carla.GearPhysicsControl.ratio"></a>**<font color="#f8805a">ratio</font>** (_float_)  
The transmission ratio of the gear.  
- <a name="carla.GearPhysicsControl.down_ratio"></a>**<font color="#f8805a">down_ratio</font>** (_float_)  
Quotient between current RPM and MaxRPM where the autonomous gear box should shift down.  
- <a name="carla.GearPhysicsControl.up_ratio"></a>**<font color="#f8805a">up_ratio</font>** (_float_)  
Quotient between current RPM and MaxRPM where the autonomous gear box should shift up.  

### Methods
- <a name="carla.GearPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**ratio**=1.0</font>, <font color="#00a6ed">**down_ratio**=0.5</font>, <font color="#00a6ed">**up_ratio**=0.65</font>)  
    - **Parameters:**
        - `ratio` (_float_)  
        - `down_ratio` (_float_)  
        - `up_ratio` (_float_)  

##### Dunder methods
- <a name="carla.GearPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.GearPhysicsControl](#carla.GearPhysicsControl)</font>)  
- <a name="carla.GearPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.GearPhysicsControl](#carla.GearPhysicsControl)</font>)  
- <a name="carla.GearPhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GeoLocation<a name="carla.GeoLocation"></a>
Class that contains geographical coordinates simulated data. The [carla.Map](#carla.Map) can convert simulation locations by using the <b><georeference></b> tag in the OpenDRIVE file.  

### Instance Variables
- <a name="carla.GeoLocation.latitude"></a>**<font color="#f8805a">latitude</font>** (_float<small> - degrees</small>_)  
North/South value of a point on the map.  
- <a name="carla.GeoLocation.longitude"></a>**<font color="#f8805a">longitude</font>** (_float<small> - degrees</small>_)  
West/East value of a point on the map.  
- <a name="carla.GeoLocation.altitude"></a>**<font color="#f8805a">altitude</font>** (_float<small> - meters</small>_)  
Height regarding ground level.  

### Methods
- <a name="carla.GeoLocation.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**latitude**=0.0</font>, <font color="#00a6ed">**longitude**=0.0</font>, <font color="#00a6ed">**altitude**=0.0</font>)  
    - **Parameters:**
        - `latitude` (_float<small> - degrees</small>_)  
        - `longitude` (_float<small> - degrees</small>_)  
        - `altitude` (_float<small> - meters</small>_)  

##### Dunder methods
- <a name="carla.GeoLocation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.GeoLocation](#carla.GeoLocation)</font>)  
- <a name="carla.GeoLocation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.GeoLocation](#carla.GeoLocation)</font>)  
- <a name="carla.GeoLocation.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GnssMeasurement<a name="carla.GnssMeasurement"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines the Gnss data registered by a <b>sensor.other.gnss</b>. It essentially reports its position with the position of the sensor and an OpenDRIVE geo-reference.  

### Instance Variables
- <a name="carla.GnssMeasurement.altitude"></a>**<font color="#f8805a">altitude</font>** (_float<small> - meters</small>_)  
Height regarding ground level.  
- <a name="carla.GnssMeasurement.latitude"></a>**<font color="#f8805a">latitude</font>** (_float<small> - degrees</small>_)  
North/South value of a point on the map.  
- <a name="carla.GnssMeasurement.longitude"></a>**<font color="#f8805a">longitude</font>** (_float<small> - degrees</small>_)  
West/East value of a point on the map.  

### Methods

##### Dunder methods
- <a name="carla.GnssMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.IMUMeasurement<a name="carla.IMUMeasurement"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines the data registered by a <b>sensor.other.imu</b>, regarding the sensor's transformation according to the current [carla.World](#carla.World). It essentially acts as accelerometer, gyroscope and compass.  

### Instance Variables
- <a name="carla.IMUMeasurement.accelerometer"></a>**<font color="#f8805a">accelerometer</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - m/s<sup>2</sup></small>_)  
Linear acceleration.  
- <a name="carla.IMUMeasurement.compass"></a>**<font color="#f8805a">compass</font>** (_float<small> - radians</small>_)  
Orientation with regard to the North ([0.0, -1.0, 0.0] in Unreal Engine).  
- <a name="carla.IMUMeasurement.gyroscope"></a>**<font color="#f8805a">gyroscope</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - rad/s</small>_)  
Angular velocity.  

### Methods

##### Dunder methods
- <a name="carla.IMUMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Image<a name="carla.Image"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines an image of 32-bit BGRA colors that will be used as initial data retrieved by camera sensors. There are different camera sensors (currently three, RGB, depth and semantic segmentation) and each of these makes different use for the images. Learn more about them [here](ref_sensors.md).  

### Instance Variables
- <a name="carla.Image.fov"></a>**<font color="#f8805a">fov</font>** (_float<small> - degrees</small>_)  
Horizontal field of view of the image.  
- <a name="carla.Image.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Image height in pixels.  
- <a name="carla.Image.width"></a>**<font color="#f8805a">width</font>** (_int_)  
Image width in pixels.  
- <a name="carla.Image.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
Flattened array of pixel data, use reshape to create an image array.  

### Methods
- <a name="carla.Image.convert"></a>**<font color="#7fb800">convert</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**color_converter**</font>)  
Converts the image following the `color_converter` pattern.  
    - **Parameters:**
        - `color_converter` (_[carla.ColorConverter](#carla.ColorConverter)_)  
- <a name="carla.Image.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>, <font color="#00a6ed">**color_converter**=Raw</font>)  
Saves the image to disk using a converter pattern stated as `color_converter`. The default conversion pattern is <b>Raw</b> that will make no changes to the image.  
    - **Parameters:**
        - `path` (_str_) - Path that will contain the image.  
        - `color_converter` (_[carla.ColorConverter](#carla.ColorConverter)_) - Default <b>Raw</b> will make no changes.  

##### Dunder methods
- <a name="carla.Image.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
- <a name="carla.Image.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.Color](#carla.Color) that form the image.  
- <a name="carla.Image.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>, <font color="#00a6ed">**color**=[carla.Color](#carla.Color)</font>)  
- <a name="carla.Image.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Junction<a name="carla.Junction"></a>
Class that embodies the intersections on the road described in the OpenDRIVE file according to OpenDRIVE 1.4 standards.  

### Instance Variables
- <a name="carla.Junction.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Identifier found in the OpenDRIVE file.  
- <a name="carla.Junction.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
Bounding box encapsulating the junction lanes.  

### Methods

##### Getters
- <a name="carla.Junction.get_waypoints"></a>**<font color="#7fb800">get_waypoints</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lane_type**</font>)  
Returns a list of pairs of waypoints. Every tuple on the list contains first an initial and then a final waypoint within the intersection boundaries that describe the beginning and the end of said lane along the junction. Lanes follow their OpenDRIVE definitions so there may be many different tuples with the same starting waypoint due to possible deviations, as this are considered different lanes.  
    - **Parameters:**
        - `lane_type` (_[carla.LaneType](#carla.LaneType)_) - Type of lanes to get the waypoints.  
    - **Return:** _list(tuple([carla.Waypoint](#carla.Waypoint)))_  

---

## carla.LabelledPoint<a name="carla.LabelledPoint"></a>
Class that represent a position in space with a semantic label.  

### Instance Variables
- <a name="carla.LabelledPoint.location"></a>**<font color="#f8805a">location</font>**  
Position in 3D space.  
- <a name="carla.LabelledPoint.label"></a>**<font color="#f8805a">label</font>**  
Semantic tag of the point.  

---

## carla.Landmark<a name="carla.Landmark"></a>
Class that defines any type of traffic landmark or sign affecting a road. These class mediates between the [OpenDRIVE 1.4 standard](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) definition of the landmarks and their representation in the simulation. This class retrieves all the information defining a landmark in OpenDRIVE and facilitates information about which lanes does it affect and when.
Landmarks will be accessed by [carla.Waypoint](#carla.Waypoint) objects trying to retrieve the regulation of their lane. Therefore some attributes depend on the waypoint that is consulting the landmark and so, creating the object.  

### Instance Variables
- <a name="carla.Landmark.road_id"></a>**<font color="#f8805a">road_id</font>** (_int_)  
The OpenDRIVE ID of the road where this landmark is defined. Due to OpenDRIVE road definitions, this road may be different from the road the landmark is currently affecting. It is mostly the case in junctions where the road diverges in different routes.
<small>Example: a traffic light is defined in one of the divergent roads in a junction, but it affects all the possible routes</small>.  
- <a name="carla.Landmark.distance"></a>**<font color="#f8805a">distance</font>** (_float<small> - meters</small>_)  
Distance between the landmark and the waypoint creating the object (querying `get_landmarks` or `get_landmarks_of_type`).  
- <a name="carla.Landmark.s"></a>**<font color="#f8805a">s</font>** (_float<small> - meters</small>_)  
Distance where the landmark is positioned along the geometry of the road `road_id`.  
- <a name="carla.Landmark.t"></a>**<font color="#f8805a">t</font>** (_float<small> - meters</small>_)  
Lateral distance where the landmark is positioned from the edge of the road `road_id`.  
- <a name="carla.Landmark.id"></a>**<font color="#f8805a">id</font>** (_str_)  
Unique ID of the landmark in the OpenDRIVE file.  
- <a name="carla.Landmark.name"></a>**<font color="#f8805a">name</font>** (_str_)  
Name of the landmark in the in the OpenDRIVE file.  
- <a name="carla.Landmark.is_dynamic"></a>**<font color="#f8805a">is_dynamic</font>** (_bool_)  
Indicates if the landmark has state changes over time such as traffic lights.  
- <a name="carla.Landmark.orientation"></a>**<font color="#f8805a">orientation</font>** (_[carla.LandmarkOrientation](#carla.LandmarkOrientation)<small> - degrees</small>_)  
Indicates which lanes the landmark is facing towards to.  
- <a name="carla.Landmark.z_offset"></a>**<font color="#f8805a">z_offset</font>** (_float<small> - meters</small>_)  
Height where the landmark is placed.  
- <a name="carla.Landmark.country"></a>**<font color="#f8805a">country</font>** (_str_)  
Country code where the landmark is defined (default to OpenDRIVE is Germany 2017).  
- <a name="carla.Landmark.type"></a>**<font color="#f8805a">type</font>** (_str_)  
Type identifier of the landmark according to the country code.  
- <a name="carla.Landmark.sub_type"></a>**<font color="#f8805a">sub_type</font>** (_str_)  
Subtype identifier of the landmark according to the country code.  
- <a name="carla.Landmark.value"></a>**<font color="#f8805a">value</font>** (_float_)  
Value printed in the signal (e.g. speed limit, maximum weight, etc).  
- <a name="carla.Landmark.unit"></a>**<font color="#f8805a">unit</font>** (_str_)  
Units of measurement for the attribute `value`.  
- <a name="carla.Landmark.height"></a>**<font color="#f8805a">height</font>** (_float<small> - meters</small>_)  
Total height of the signal.  
- <a name="carla.Landmark.width"></a>**<font color="#f8805a">width</font>** (_float<small> - meters</small>_)  
Total width of the signal.  
- <a name="carla.Landmark.text"></a>**<font color="#f8805a">text</font>** (_str_)  
Additional text in the signal.  
- <a name="carla.Landmark.h_offset"></a>**<font color="#f8805a">h_offset</font>** (_float<small> - meters</small>_)  
Orientation offset of the signal relative to the the definition of `road_id` at `s` in OpenDRIVE.  
- <a name="carla.Landmark.pitch"></a>**<font color="#f8805a">pitch</font>** (_float<small> - meters</small>_)  
Pitch rotation of the signal (Y-axis in [UE coordinates system](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/python_api/#carlarotation)).  
- <a name="carla.Landmark.roll"></a>**<font color="#f8805a">roll</font>** (_float_)  
Roll rotation of the signal (X-axis in [UE coordinates system](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/python_api/#carlarotation)).  
- <a name="carla.Landmark.waypoint"></a>**<font color="#f8805a">waypoint</font>** (_[carla.Waypoint](#carla.Waypoint)_)  
A waypoint placed in the lane of the one that made the query and at the `s` of the landmark. It is the first waypoint for which the landmark will be effective.  
- <a name="carla.Landmark.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
The location and orientation of the landmark in the simulation.  

### Methods

##### Getters
- <a name="carla.Landmark.get_lane_validities"></a>**<font color="#7fb800">get_lane_validities</font>**(<font color="#00a6ed">**self**</font>)  
Returns which lanes the landmark is affecting to. As there may be specific lanes where the landmark is not effective, the return is a list of pairs containing ranges of the __lane_id__ affected:
<small>Example: In a road with 5 lanes, being 3 not affected: [(from_lane1,to_lane2),(from_lane4,to_lane5)]</small>.  
    - **Return:** _list(tuple(int))_  

---

## carla.LandmarkOrientation<a name="carla.LandmarkOrientation"></a>
Helper class to define the orientation of a landmark in the road. The definition is not directly translated from OpenDRIVE but converted for the sake of understanding.  

### Instance Variables
- <a name="carla.LandmarkOrientation.Positive"></a>**<font color="#f8805a">Positive</font>**  
The landmark faces towards vehicles going on the same direction as the road's geometry definition (lanes 0 and negative in OpenDRIVE).  
- <a name="carla.LandmarkOrientation.Negative"></a>**<font color="#f8805a">Negative</font>**  
The landmark faces towards vehicles going on the opposite direction to the road's geometry definition (positive lanes in OpenDRIVE).  
- <a name="carla.LandmarkOrientation.Both"></a>**<font color="#f8805a">Both</font>**  
Affects vehicles going in both directions of the road.  

---

## carla.LandmarkType<a name="carla.LandmarkType"></a>
Helper class containing a set of commonly used landmark types as defined by the default country code in the [OpenDRIVE standard](http://opendrive.org/docs/OpenDRIVEFormatSpecRev1.5M.pdf) (Germany 2017).
__[carla.Landmark](#carla.Landmark) does not reference this class__. The landmark type is a string that varies greatly depending on the country code being used. This class only makes it easier to manage some of the most commonly used in the default set by describing them as an enum.  

### Instance Variables
- <a name="carla.LandmarkType.Danger"></a>**<font color="#f8805a">Danger</font>**  
Type 101.  
- <a name="carla.LandmarkType.LanesMerging"></a>**<font color="#f8805a">LanesMerging</font>**  
Type 121.  
- <a name="carla.LandmarkType.CautionPedestrian"></a>**<font color="#f8805a">CautionPedestrian</font>**  
Type 133.  
- <a name="carla.LandmarkType.CautionBicycle"></a>**<font color="#f8805a">CautionBicycle</font>**  
Type 138.  
- <a name="carla.LandmarkType.LevelCrossing"></a>**<font color="#f8805a">LevelCrossing</font>**  
Type 150.  
- <a name="carla.LandmarkType.StopSign"></a>**<font color="#f8805a">StopSign</font>**  
Type 206.  
- <a name="carla.LandmarkType.YieldSign"></a>**<font color="#f8805a">YieldSign</font>**  
Type 205.  
- <a name="carla.LandmarkType.MandatoryTurnDirection"></a>**<font color="#f8805a">MandatoryTurnDirection</font>**  
Type 209.  
- <a name="carla.LandmarkType.MandatoryLeftRightDirection"></a>**<font color="#f8805a">MandatoryLeftRightDirection</font>**  
Type 211.  
- <a name="carla.LandmarkType.TwoChoiceTurnDirection"></a>**<font color="#f8805a">TwoChoiceTurnDirection</font>**  
Type 214.  
- <a name="carla.LandmarkType.Roundabout"></a>**<font color="#f8805a">Roundabout</font>**  
Type 215.  
- <a name="carla.LandmarkType.PassRightLeft"></a>**<font color="#f8805a">PassRightLeft</font>**  
Type 222.  
- <a name="carla.LandmarkType.AccessForbidden"></a>**<font color="#f8805a">AccessForbidden</font>**  
Type 250.  
- <a name="carla.LandmarkType.AccessForbiddenMotorvehicles"></a>**<font color="#f8805a">AccessForbiddenMotorvehicles</font>**  
Type 251.  
- <a name="carla.LandmarkType.AccessForbiddenTrucks"></a>**<font color="#f8805a">AccessForbiddenTrucks</font>**  
Type 253.  
- <a name="carla.LandmarkType.AccessForbiddenBicycle"></a>**<font color="#f8805a">AccessForbiddenBicycle</font>**  
Type 254.  
- <a name="carla.LandmarkType.AccessForbiddenWeight"></a>**<font color="#f8805a">AccessForbiddenWeight</font>**  
Type 263.  
- <a name="carla.LandmarkType.AccessForbiddenWidth"></a>**<font color="#f8805a">AccessForbiddenWidth</font>**  
Type 264.  
- <a name="carla.LandmarkType.AccessForbiddenHeight"></a>**<font color="#f8805a">AccessForbiddenHeight</font>**  
Type 265.  
- <a name="carla.LandmarkType.AccessForbiddenWrongDirection"></a>**<font color="#f8805a">AccessForbiddenWrongDirection</font>**  
Type 267.  
- <a name="carla.LandmarkType.ForbiddenUTurn"></a>**<font color="#f8805a">ForbiddenUTurn</font>**  
Type 272.  
- <a name="carla.LandmarkType.MaximumSpeed"></a>**<font color="#f8805a">MaximumSpeed</font>**  
Type 274.  
- <a name="carla.LandmarkType.ForbiddenOvertakingMotorvehicles"></a>**<font color="#f8805a">ForbiddenOvertakingMotorvehicles</font>**  
Type 276.  
- <a name="carla.LandmarkType.ForbiddenOvertakingTrucks"></a>**<font color="#f8805a">ForbiddenOvertakingTrucks</font>**  
Type 277.  
- <a name="carla.LandmarkType.AbsoluteNoStop"></a>**<font color="#f8805a">AbsoluteNoStop</font>**  
Type 283.  
- <a name="carla.LandmarkType.RestrictedStop"></a>**<font color="#f8805a">RestrictedStop</font>**  
Type 286.  
- <a name="carla.LandmarkType.HasWayNextIntersection"></a>**<font color="#f8805a">HasWayNextIntersection</font>**  
Type 301.  
- <a name="carla.LandmarkType.PriorityWay"></a>**<font color="#f8805a">PriorityWay</font>**  
Type 306.  
- <a name="carla.LandmarkType.PriorityWayEnd"></a>**<font color="#f8805a">PriorityWayEnd</font>**  
Type 307.  
- <a name="carla.LandmarkType.CityBegin"></a>**<font color="#f8805a">CityBegin</font>**  
Type 310.  
- <a name="carla.LandmarkType.CityEnd"></a>**<font color="#f8805a">CityEnd</font>**  
Type 311.  
- <a name="carla.LandmarkType.Highway"></a>**<font color="#f8805a">Highway</font>**  
Type 330.  
- <a name="carla.LandmarkType.DeadEnd"></a>**<font color="#f8805a">DeadEnd</font>**  
Type 357.  
- <a name="carla.LandmarkType.RecomendedSpeed"></a>**<font color="#f8805a">RecomendedSpeed</font>**  
Type 380.  
- <a name="carla.LandmarkType.RecomendedSpeedEnd"></a>**<font color="#f8805a">RecomendedSpeedEnd</font>**  
Type 381.  

---

## carla.LaneChange<a name="carla.LaneChange"></a>
Class that defines the permission to turn either left, right, both or none (meaning only going straight is allowed). This information is stored for every [carla.Waypoint](#carla.Waypoint) according to the OpenDRIVE file. The snipet in [carla.Map.get_waypoint](#carla.Map.get_waypoint) shows how a waypoint can be used to learn which turns are permitted.  

### Instance Variables
- <a name="carla.LaneChange.NONE"></a>**<font color="#f8805a">NONE</font>**  
Traffic rules do not allow turning right or left, only going straight.  
- <a name="carla.LaneChange.Right"></a>**<font color="#f8805a">Right</font>**  
Traffic rules allow turning right.  
- <a name="carla.LaneChange.Left"></a>**<font color="#f8805a">Left</font>**  
Traffic rules allow turning left.  
- <a name="carla.LaneChange.Both"></a>**<font color="#f8805a">Both</font>**  
Traffic rules allow turning either right or left.  

---

## carla.LaneInvasionEvent<a name="carla.LaneInvasionEvent"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines lanes invasion for <b>sensor.other.lane_invasion</b>. It works only client-side and is dependant on OpenDRIVE to provide reliable information. The sensor creates one of this every time there is a lane invasion, which may be more than once per simulation step. Learn more about this [here](ref_sensors.md#lane-invasion-detector).  

### Instance Variables
- <a name="carla.LaneInvasionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Gets the actor the sensor is attached to, the one that invaded another lane.  
- <a name="carla.LaneInvasionEvent.crossed_lane_markings"></a>**<font color="#f8805a">crossed_lane_markings</font>** (_list([carla.LaneMarking](#carla.LaneMarking))_)  
List of lane markings that have been crossed and detected by the sensor.  

### Methods

##### Dunder methods
- <a name="carla.LaneInvasionEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.LaneMarking<a name="carla.LaneMarking"></a>
Class that gathers all the information regarding a lane marking according to [OpenDRIVE 1.4 standard](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) standard.  

### Instance Variables
- <a name="carla.LaneMarking.color"></a>**<font color="#f8805a">color</font>** (_[carla.LaneMarkingColor](#carla.LaneMarkingColor)_)  
Actual color of the marking.  
- <a name="carla.LaneMarking.lane_change"></a>**<font color="#f8805a">lane_change</font>** (_[carla.LaneChange](#carla.LaneChange)_)  
Permissions for said lane marking to be crossed.  
- <a name="carla.LaneMarking.type"></a>**<font color="#f8805a">type</font>** (_[carla.LaneMarkingType](#carla.LaneMarkingType)_)  
Lane marking type.  
- <a name="carla.LaneMarking.width"></a>**<font color="#f8805a">width</font>** (_float_)  
Horizontal lane marking thickness.  

---

## carla.LaneMarkingColor<a name="carla.LaneMarkingColor"></a>
Class that defines the lane marking colors according to OpenDRIVE 1.4.  

### Instance Variables
- <a name="carla.LaneMarkingColor.Standard"></a>**<font color="#f8805a">Standard</font>**  
White by default.  
- <a name="carla.LaneMarkingColor.Blue"></a>**<font color="#f8805a">Blue</font>**  
- <a name="carla.LaneMarkingColor.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.LaneMarkingColor.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.LaneMarkingColor.White"></a>**<font color="#f8805a">White</font>**  
- <a name="carla.LaneMarkingColor.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.LaneMarkingColor.Other"></a>**<font color="#f8805a">Other</font>**  

---

## carla.LaneMarkingType<a name="carla.LaneMarkingType"></a>
Class that defines the lane marking types accepted by OpenDRIVE 1.4. The snipet in [carla.Map.get_waypoint](#carla.Map.get_waypoint) shows how a waypoint can be used to retrieve the information about adjacent lane markings.   <br><br> __Note on double types:__ Lane markings are defined under the OpenDRIVE standard that determines whereas a line will be considered "BrokenSolid" or "SolidBroken". For each road there is a center lane marking, defined from left to right regarding the lane's directions. The rest of the lane markings are defined in order from the center lane to the closest outside of the road.  

### Instance Variables
- <a name="carla.LaneMarkingType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneMarkingType.Other"></a>**<font color="#f8805a">Other</font>**  
- <a name="carla.LaneMarkingType.Broken"></a>**<font color="#f8805a">Broken</font>**  
- <a name="carla.LaneMarkingType.Solid"></a>**<font color="#f8805a">Solid</font>**  
- <a name="carla.LaneMarkingType.SolidSolid"></a>**<font color="#f8805a">SolidSolid</font>**  
- <a name="carla.LaneMarkingType.SolidBroken"></a>**<font color="#f8805a">SolidBroken</font>**  
- <a name="carla.LaneMarkingType.BrokenSolid"></a>**<font color="#f8805a">BrokenSolid</font>**  
- <a name="carla.LaneMarkingType.BrokenBroken"></a>**<font color="#f8805a">BrokenBroken</font>**  
- <a name="carla.LaneMarkingType.BottsDots"></a>**<font color="#f8805a">BottsDots</font>**  
- <a name="carla.LaneMarkingType.Grass"></a>**<font color="#f8805a">Grass</font>**  
- <a name="carla.LaneMarkingType.Curb"></a>**<font color="#f8805a">Curb</font>**  

---

## carla.LaneType<a name="carla.LaneType"></a>
Class that defines the possible lane types accepted by OpenDRIVE 1.4. This standards define the road information. The snipet in [carla.Map.get_waypoint](#carla.Map.get_waypoint) makes use of a waypoint to get the current and adjacent lane types.  

### Instance Variables
- <a name="carla.LaneType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneType.Driving"></a>**<font color="#f8805a">Driving</font>**  
- <a name="carla.LaneType.Stop"></a>**<font color="#f8805a">Stop</font>**  
- <a name="carla.LaneType.Shoulder"></a>**<font color="#f8805a">Shoulder</font>**  
- <a name="carla.LaneType.Biking"></a>**<font color="#f8805a">Biking</font>**  
- <a name="carla.LaneType.Sidewalk"></a>**<font color="#f8805a">Sidewalk</font>**  
- <a name="carla.LaneType.Border"></a>**<font color="#f8805a">Border</font>**  
- <a name="carla.LaneType.Restricted"></a>**<font color="#f8805a">Restricted</font>**  
- <a name="carla.LaneType.Parking"></a>**<font color="#f8805a">Parking</font>**  
- <a name="carla.LaneType.Bidirectional"></a>**<font color="#f8805a">Bidirectional</font>**  
- <a name="carla.LaneType.Median"></a>**<font color="#f8805a">Median</font>**  
- <a name="carla.LaneType.Special1"></a>**<font color="#f8805a">Special1</font>**  
- <a name="carla.LaneType.Special2"></a>**<font color="#f8805a">Special2</font>**  
- <a name="carla.LaneType.Special3"></a>**<font color="#f8805a">Special3</font>**  
- <a name="carla.LaneType.RoadWorks"></a>**<font color="#f8805a">RoadWorks</font>**  
- <a name="carla.LaneType.Tram"></a>**<font color="#f8805a">Tram</font>**  
- <a name="carla.LaneType.Rail"></a>**<font color="#f8805a">Rail</font>**  
- <a name="carla.LaneType.Entry"></a>**<font color="#f8805a">Entry</font>**  
- <a name="carla.LaneType.Exit"></a>**<font color="#f8805a">Exit</font>**  
- <a name="carla.LaneType.OffRamp"></a>**<font color="#f8805a">OffRamp</font>**  
- <a name="carla.LaneType.OnRamp"></a>**<font color="#f8805a">OnRamp</font>**  
- <a name="carla.LaneType.Any"></a>**<font color="#f8805a">Any</font>**  
Every type except for NONE.  

---

## carla.LidarDetection<a name="carla.LidarDetection"></a>
Data contained inside a [carla.LidarMeasurement](#carla.LidarMeasurement). Each of these represents one of the points in the cloud with its location and its associated intensity.  

### Instance Variables
- <a name="carla.LidarDetection.point"></a>**<font color="#f8805a">point</font>** (_[carla.Location](#carla.Location)<small> - meters</small>_)  
Point in xyz coordinates.  
- <a name="carla.LidarDetection.intensity"></a>**<font color="#f8805a">intensity</font>** (_float_)  
Computed intensity for this point as a scalar value between [0.0 , 1.0].  

### Methods

##### Dunder methods
- <a name="carla.LidarDetection.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.LidarMeasurement<a name="carla.LidarMeasurement"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines the LIDAR data retrieved by a <b>sensor.lidar.ray_cast</b>. This essentially simulates a rotating LIDAR using ray-casting. Learn more about this [here](ref_sensors.md#lidar-raycast-sensor).  

### Instance Variables
- <a name="carla.LidarMeasurement.channels"></a>**<font color="#f8805a">channels</font>** (_int_)  
Number of lasers shot.  
- <a name="carla.LidarMeasurement.horizontal_angle"></a>**<font color="#f8805a">horizontal_angle</font>** (_float<small> - radians</small>_)  
Horizontal angle the LIDAR is rotated at the time of the measurement.  
- <a name="carla.LidarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
Received list of 4D points. Each point consists of [x,y,z] coordinates plus the intensity computed for that point.  

### Methods
- <a name="carla.LidarMeasurement.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Saves the point cloud to disk as a <b>.ply</b> file describing data from 3D scanners. The files generated are ready to be used within [MeshLab](http://www.meshlab.net/), an open source system for processing said files. Just take into account that axis may differ from Unreal Engine and so, need to be reallocated.  
    - **Parameters:**
        - `path` (_str_)  

##### Getters
- <a name="carla.LidarMeasurement.get_point_count"></a>**<font color="#7fb800">get_point_count</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**channel**</font>)  
Retrieves the number of points sorted by channel that are generated by this measure. Sorting by channel allows to identify the original channel for every point.  
    - **Parameters:**
        - `channel` (_int_)  

##### Dunder methods
- <a name="carla.LidarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
- <a name="carla.LidarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.LidarDetection](#carla.LidarDetection) retrieved as data.  
- <a name="carla.LidarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>, <font color="#00a6ed">**detection**=[carla.LidarDetection](#carla.LidarDetection)</font>)  
- <a name="carla.LidarMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Light<a name="carla.Light"></a>
This class exposes the lights that exist in the scene, except for vehicle lights. The properties of a light can be queried and changed at will.
Lights are automatically turned on when the simulator enters night mode (sun altitude is below zero).  

### Instance Variables
- <a name="carla.Light.color"></a>**<font color="#f8805a">color</font>** (_[carla.Color](#carla.Color)_)  
Color of the light.  
- <a name="carla.Light.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Identifier of the light.  
- <a name="carla.Light.intensity"></a>**<font color="#f8805a">intensity</font>** (_float<small> - lumens</small>_)  
Intensity of the light.  
- <a name="carla.Light.is_on"></a>**<font color="#f8805a">is_on</font>** (_bool_)  
Switch of the light. It is __True__ when the light is on. When the night mode starts, this is set to __True__.  
- <a name="carla.Light.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)<small> - meters</small>_)  
Position of the light.  
- <a name="carla.Light.light_group"></a>**<font color="#f8805a">light_group</font>** (_[carla.LightGroup](#carla.LightGroup)_)  
Group the light belongs to.  
- <a name="carla.Light.light_state"></a>**<font color="#f8805a">light_state</font>** (_[carla.LightState](#carla.LightState)_)  
State of the light. Summarizes its attributes, group, and if it is on/off.  

### Methods
- <a name="carla.Light.turn_off"></a>**<font color="#7fb800">turn_off</font>**(<font color="#00a6ed">**self**</font>)  
Switches off the light.  
- <a name="carla.Light.turn_on"></a>**<font color="#7fb800">turn_on</font>**(<font color="#00a6ed">**self**</font>)  
Switches on the light.  

##### Setters
- <a name="carla.Light.set_color"></a>**<font color="#7fb800">set_color</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**color**</font>)  
Changes the color of the light to `color`.  
    - **Parameters:**
        - `color` (_[carla.Color](#carla.Color)_)  
- <a name="carla.Light.set_intensity"></a>**<font color="#7fb800">set_intensity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**intensity**</font>)  
Changes the intensity of the light to `intensity`.  
    - **Parameters:**
        - `intensity` (_float<small> - lumens</small>_)  
- <a name="carla.Light.set_light_group"></a>**<font color="#7fb800">set_light_group</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**light_group**</font>)  
Changes the light to the group `light_group`.  
    - **Parameters:**
        - `light_group` (_[carla.LightGroup](#carla.LightGroup)_)  
- <a name="carla.Light.set_light_state"></a>**<font color="#7fb800">set_light_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**light_state**</font>)  
Changes the state of the light to `light_state`. This may change attributes, group and turn the light on/off all at once.  
    - **Parameters:**
        - `light_state` (_[carla.LightState](#carla.LightState)_)  

---

## carla.LightGroup<a name="carla.LightGroup"></a>
This class categorizes the lights on scene into different groups. These groups available are provided as a enum values that can be used as flags.  

__Note.__ So far, though there is a `vehicle` group, vehicle lights are not available as [carla.Light](#carla.Light) objects. These have to be managed using [carla.Vehicle](#carla.Vehicle) and [carla.VehicleLightState](#carla.VehicleLightState).  

### Instance Variables
- <a name="carla.LightGroup.None"></a>**<font color="#f8805a">None</font>**  
All lights.  
- <a name="carla.LightGroup.Vehicle"></a>**<font color="#f8805a">Vehicle</font>**  
- <a name="carla.LightGroup.Street"></a>**<font color="#f8805a">Street</font>**  
- <a name="carla.LightGroup.Building"></a>**<font color="#f8805a">Building</font>**  
- <a name="carla.LightGroup.Other"></a>**<font color="#f8805a">Other</font>**  

---

## carla.LightManager<a name="carla.LightManager"></a>
This class handles the lights in the scene. Its main use is to get and set the state of groups or lists of lights in one call. An instance of this class can be retrieved by the [carla.World.get_lightmanager](#carla.World.get_lightmanager)().  

__Note.__ So far, though there is a `vehicle` group, vehicle lights are not available as [carla.Light](#carla.Light) objects. These have to be managed using [carla.Vehicle](#carla.Vehicle) and [carla.VehicleLightState](#carla.VehicleLightState).  

### Methods
- <a name="carla.LightManager.is_active"></a>**<font color="#7fb800">is_active</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Returns a list with booleans stating if the elements in `lights` are switched on/off.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be queried.  
    - **Return:** _list(bool)_  
- <a name="carla.LightManager.turn_off"></a>**<font color="#7fb800">turn_off</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Switches off all the lights in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be switched off.  
- <a name="carla.LightManager.turn_on"></a>**<font color="#7fb800">turn_on</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Switches on all the lights in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be switched on.  

##### Getters
- <a name="carla.LightManager.get_all_lights"></a>**<font color="#7fb800">get_all_lights</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**light_group**=[carla.LightGroup.None](#carla.LightGroup.None)</font>)  
Returns a list containing the lights in a certain group. By default, the group is `None`.  
    - **Parameters:**
        - `light_group` (_[carla.LightGroup](#carla.LightGroup)_) - Group to filter the lights returned. Default is `None`.  
    - **Return:** _list([carla.Light](#carla.Light))_  
- <a name="carla.LightManager.get_color"></a>**<font color="#7fb800">get_color</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Returns a list with the colors of every element in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be queried.  
    - **Return:** _list([carla.Color](#carla.Color))_  
    - **Setter:** _[carla.LightManager.set_color](#carla.LightManager.set_color)_  
- <a name="carla.LightManager.get_intensity"></a>**<font color="#7fb800">get_intensity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Returns a list with the intensity of every element in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be queried.  
    - **Return:** _list(float)<small> - lumens</small>_  
    - **Setter:** _[carla.LightManager.set_intensity](#carla.LightManager.set_intensity)_  
- <a name="carla.LightManager.get_light_group"></a>**<font color="#7fb800">get_light_group</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Returns a list with the group of every element in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be queried.  
    - **Return:** _list([carla.LightGroup](#carla.LightGroup))_  
    - **Setter:** _[carla.LightManager.set_light_group](#carla.LightManager.set_light_group)_  
- <a name="carla.LightManager.get_light_state"></a>**<font color="#7fb800">get_light_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>)  
Returns a list with the state of all the attributes of every element in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be queried.  
    - **Return:** _list([carla.LightState](#carla.LightState))_  
    - **Setter:** _[carla.LightManager.set_light_state](#carla.LightManager.set_light_state)_  
- <a name="carla.LightManager.get_turned_off_lights"></a>**<font color="#7fb800">get_turned_off_lights</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**light_group**</font>)  
Returns a list containing lights switched off in the scene, filtered by group.  
    - **Parameters:**
        - `light_group` (_[carla.LightGroup](#carla.LightGroup)_) - List of lights to be queried.  
    - **Return:** _list([carla.Light](#carla.Light))_  
- <a name="carla.LightManager.get_turned_on_lights"></a>**<font color="#7fb800">get_turned_on_lights</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**light_group**</font>)  
Returns a list containing lights switched on in the scene, filtered by group.  
    - **Parameters:**
        - `light_group` (_[carla.LightGroup](#carla.LightGroup)_) - List of lights to be queried.  
    - **Return:** _list([carla.Light](#carla.Light))_  

##### Setters
- <a name="carla.LightManager.set_active"></a>**<font color="#7fb800">set_active</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**active**</font>)  
Switches on/off the elements in `lights`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be switched on/off.  
        - `active` (_list(bool)_) - List of booleans to be applied.  
- <a name="carla.LightManager.set_color"></a>**<font color="#7fb800">set_color</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**color**</font>)  
Changes the color of the elements in `lights` to `color`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `color` (_[carla.Color](#carla.Color)_) - Color to be applied.  
    - **Getter:** _[carla.LightManager.get_color](#carla.LightManager.get_color)_  
- <a name="carla.LightManager.set_colors"></a>**<font color="#7fb800">set_colors</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**colors**</font>)  
Changes the color of each element in `lights` to the corresponding in `colors`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `colors` (_list([carla.Color](#carla.Color))_) - List of colors to be applied.  
- <a name="carla.LightManager.set_day_night_cycle"></a>**<font color="#7fb800">set_day_night_cycle</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**active**</font>)  
All scene lights have a day-night cycle, automatically turning on and off with the altitude of the sun. This interferes in cases where full control of the scene lights is required, so setting this to __False__ deactivates it. It can reactivated by setting it to __True__.  
    - **Parameters:**
        - `active` (_bool_) - (De)activation of the day-night cycle.  
- <a name="carla.LightManager.set_intensities"></a>**<font color="#7fb800">set_intensities</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**intensities**</font>)  
Changes the intensity of each element in `lights` to the corresponding in `intensities`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `intensities` (_list(float)<small> - lumens</small>_) - List of intensities to be applied.  
- <a name="carla.LightManager.set_intensity"></a>**<font color="#7fb800">set_intensity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**intensity**</font>)  
Changes the intensity of every element in `lights` to `intensity`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `intensity` (_float<small> - lumens</small>_) - Intensity to be applied.  
    - **Getter:** _[carla.LightManager.get_intensity](#carla.LightManager.get_intensity)_  
- <a name="carla.LightManager.set_light_group"></a>**<font color="#7fb800">set_light_group</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**light_group**</font>)  
Changes the group of every element in `lights` to `light_group`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `light_group` (_[carla.LightGroup](#carla.LightGroup)_) - Group to be applied.  
    - **Getter:** _[carla.LightManager.get_light_group](#carla.LightManager.get_light_group)_  
- <a name="carla.LightManager.set_light_groups"></a>**<font color="#7fb800">set_light_groups</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**light_groups**</font>)  
Changes the group of each element in `lights` to the corresponding in `light_groups`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `light_groups` (_list([carla.LightGroup](#carla.LightGroup))_) - List of groups to be applied.  
- <a name="carla.LightManager.set_light_state"></a>**<font color="#7fb800">set_light_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**light_state**</font>)  
Changes the state of the attributes of every element in `lights` to `light_state`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `light_state` (_[carla.LightState](#carla.LightState)_) - State of the attributes to be applied.  
    - **Getter:** _[carla.LightManager.get_light_state](#carla.LightManager.get_light_state)_  
- <a name="carla.LightManager.set_light_states"></a>**<font color="#7fb800">set_light_states</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lights**</font>, <font color="#00a6ed">**light_states**</font>)  
Changes the state of the attributes of each element in `lights` to the corresponding in `light_states`.  
    - **Parameters:**
        - `lights` (_list([carla.Light](#carla.Light))_) - List of lights to be changed.  
        - `light_states` (_list([carla.LightState](#carla.LightState))_) - List of state of the attributes to be applied.  

---

## carla.LightState<a name="carla.LightState"></a>
This class represents all the light variables except the identifier and the location, which are should to be static. Using this class allows to manage all the parametrization of the light in one call.  

### Instance Variables
- <a name="carla.LightState.intensity"></a>**<font color="#f8805a">intensity</font>** (_float<small> - lumens</small>_)  
Intensity of a light.  
- <a name="carla.LightState.color"></a>**<font color="#f8805a">color</font>** (_[carla.Color](#carla.Color)_)  
Color of a light.  
- <a name="carla.LightState.group"></a>**<font color="#f8805a">group</font>** (_[carla.LightGroup](#carla.LightGroup)_)  
Group a light belongs to.  
- <a name="carla.LightState.active"></a>**<font color="#f8805a">active</font>** (_bool_)  
Switch of a light. It is __True__ when the light is on.  

### Methods
- <a name="carla.LightState.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**intensity**=0.0</font>, <font color="#00a6ed">**color**=[carla.Color](#carla.Color)()</font>, <font color="#00a6ed">**group**=[carla.LightGroup.None](#carla.LightGroup.None)</font>, <font color="#00a6ed">**active**=False</font>)  
    - **Parameters:**
        - `intensity` (_float<small> - lumens</small>_) - Intensity of the light. Default is `0.0`.  
        - `color` (_[carla.Color](#carla.Color)_) - Color of the light. Default is black.  
        - `group` (_[carla.LightGroup](#carla.LightGroup)_) - Group the light belongs to. Default is the generic group `None`.  
        - `active` (_bool_) - Swith of the light. Default is `False`, light is off.  

---

## carla.Location<a name="carla.Location"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Vector3D](#carla.Vector3D)_</small></br>
Represents a spot in the world.  

### Instance Variables
- <a name="carla.Location.x"></a>**<font color="#f8805a">x</font>** (_float<small> - meters</small>_)  
Distance from origin to spot on X axis.  
- <a name="carla.Location.y"></a>**<font color="#f8805a">y</font>** (_float<small> - meters</small>_)  
Distance from origin to spot on Y axis.  
- <a name="carla.Location.z"></a>**<font color="#f8805a">z</font>** (_float<small> - meters</small>_)  
Distance from origin to spot on Z axis.  

### Methods
- <a name="carla.Location.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>, <font color="#00a6ed">**z**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
        - `z` (_float_)  
- <a name="carla.Location.distance"></a>**<font color="#7fb800">distance</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Returns Euclidean distance from this location to another one.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) - The other point to compute the distance with.  
    - **Return:** _float<small> - meters</small>_  

##### Dunder methods
- <a name="carla.Location.__abs__"></a>**<font color="#7fb800">\__abs__</font>**(<font color="#00a6ed">**self**</font>)  
Returns a Location with the absolute value of the components x, y and z.  
    - **Return:** _[carla.Location](#carla.Location)_  
- <a name="carla.Location.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Location](#carla.Location)</font>)  
Returns __True__ if both locations are the same point in space.  
    - **Return:** _bool_  
- <a name="carla.Location.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Location](#carla.Location)</font>)  
Returns __True__ if both locations are different points in space.  
    - **Return:** _bool_  
- <a name="carla.Location.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses the axis' values to string.  
    - **Return:** _str_  

---

## carla.Map<a name="carla.Map"></a>
Class containing the road information and waypoint managing. Data is retrieved from an OpenDRIVE file that describes the road. A query system is defined which works hand in hand with [carla.Waypoint](#carla.Waypoint) to translate geometrical information from the .xodr to natural world points. CARLA is currently working with [OpenDRIVE 1.4 standard](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf).  

### Instance Variables
- <a name="carla.Map.name"></a>**<font color="#f8805a">name</font>** (_str_)  
The name of the map. It corresponds to the .umap from Unreal Engine that is loaded from a CARLA server, which then references to the .xodr road description.  

### Methods
- <a name="carla.Map.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**xodr_content**</font>)  
Constructor for this class. Though a map is automatically generated when initializing the world, using this method in no-rendering mode facilitates working with an .xodr without any CARLA server running.  
    - **Parameters:**
        - `name` (_str_) - Name of the current map.  
        - `xodr_content` (_str_) - .xodr content in string format.  
    - **Return:** _list([carla.Transform](#carla.Transform))_  
- <a name="carla.Map.cook_in_memory_map"></a>**<font color="#7fb800">cook_in_memory_map</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Generates a binary file from the CARLA map containing information used by the Traffic Manager. This method is only used during the import process for maps.  
    - **Parameters:**
        - `path` (_str_) - Path to the intended location of the stored binary map file.  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#7fb800">generate_waypoints</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints with a certain distance between them for every lane and centered inside of it. Waypoints are not listed in any particular order. Remember that waypoints closer than 2cm within the same road, section and lane will have the same identificator.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - Approximate distance between waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Map.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Saves the .xodr OpenDRIVE file of the current map to disk.  
    - **Parameters:**
        - `path` - Path where the file will be saved.  
- <a name="carla.Map.to_opendrive"></a>**<font color="#7fb800">to_opendrive</font>**(<font color="#00a6ed">**self**</font>)  
Returns the .xodr OpenDRIVe file of the current map as string.  
    - **Return:** _str_  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#7fb800">transform_to_geolocation</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Converts a given `location`, a point in the simulation, to a [carla.GeoLocation](#carla.GeoLocation), which represents world coordinates. The geographical location of the map is defined inside OpenDRIVE within the tag <b><georeference></b>.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
    - **Return:** _[carla.GeoLocation](#carla.GeoLocation)_  

##### Getters
- <a name="carla.Map.get_all_landmarks"></a>**<font color="#7fb800">get_all_landmarks</font>**(<font color="#00a6ed">**self**</font>)  
Returns all the landmarks in the map. Landmarks retrieved using this method have a __null__ waypoint.  
    - **Return:** _list([carla.Landmark](#carla.Landmark))_  
- <a name="carla.Map.get_all_landmarks_from_id"></a>**<font color="#7fb800">get_all_landmarks_from_id</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**opendrive_id**</font>)  
Returns the landmarks with a certain OpenDRIVE ID. Landmarks retrieved using this method have a __null__ waypoint.  
    - **Parameters:**
        - `opendrive_id` (_string_) - The OpenDRIVE ID of the landmarks.  
    - **Return:** _list([carla.Landmark](#carla.Landmark))_  
- <a name="carla.Map.get_all_landmarks_of_type"></a>**<font color="#7fb800">get_all_landmarks_of_type</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**type**</font>)  
Returns the landmarks of a specific type. Landmarks retrieved using this method have a __null__ waypoint.  
    - **Parameters:**
        - `type` (_string_) - The type of the landmarks.  
    - **Return:** _list([carla.Landmark](#carla.Landmark))_  
- <a name="carla.Map.get_crosswalks"></a>**<font color="#7fb800">get_crosswalks</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of locations with all crosswalk zones in the form of closed polygons. The first point is repeated, symbolizing where the polygon begins and ends.  
    - **Return:** _list([carla.Location](#carla.Location))_  
- <a name="carla.Map.get_landmark_group"></a>**<font color="#7fb800">get_landmark_group</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**landmark**</font>)  
Returns the landmarks in the same group as the specified landmark (including itself). Returns an empty list if the landmark does not belong to any group.  
    - **Parameters:**
        - `landmark` (_[carla.Landmark](#carla.Landmark)_) - A landmark that belongs to the group.  
    - **Return:** _list([carla.Landmark](#carla.Landmark))_  
- <a name="carla.Map.get_spawn_points"></a>**<font color="#7fb800">get_spawn_points</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of recommendations made by the creators of the map to be used as spawning points for the vehicles. The list includes [carla.Transform](#carla.Transform) objects with certain location and orientation. Said locations are slightly on-air in order to avoid Z-collisions, so vehicles fall for a bit before starting their way.  
    - **Return:** _list([carla.Transform](#carla.Transform))_  
- <a name="carla.Map.get_topology"></a>**<font color="#7fb800">get_topology</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of tuples describing a minimal graph of the topology of the OpenDRIVE file. The tuples contain pairs of waypoints located either at the point a road begins or ends. The first one is the origin and the second one represents another road end that can be reached. This graph can be loaded into [NetworkX](https://networkx.github.io/) to work with. Output could look like this: <b>[(w0, w1), (w0, w2), (w1, w3), (w2, w3), (w0, w4)]</b>.  
    - **Return:** _list(tuple([carla.Waypoint](#carla.Waypoint), [carla.Waypoint](#carla.Waypoint)))_  
- <a name="carla.Map.get_waypoint"></a>**<font color="#7fb800">get_waypoint</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**project_to_road**=True</font>, <font color="#00a6ed">**lane_type**=[carla.LaneType.Driving](#carla.LaneType.Driving)</font>)<button class="SnipetButton" id="carla.Map.get_waypoint-snipet_button">snippet &rarr;</button>  
Returns a waypoint that can be located in an exact location or translated to the center of the nearest lane. Said lane type can be defined using flags such as `LaneType.Driving & LaneType.Shoulder`.
 The method will return <b>None</b> if the waypoint is not found, which may happen only when trying to retrieve a waypoint for an exact location. That eases checking if a point is inside a certain road, as otherwise, it will return the corresponding waypoint.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)<small> - meters</small>_) - Location used as reference for the [carla.Waypoint](#carla.Waypoint).  
        - `project_to_road` (_bool_) - If **True**, the waypoint will be at the center of the closest lane. This is the default setting. If **False**, the waypoint will be exactly in `location`. <b>None</b> means said location does not belong to a road.  
        - `lane_type` (_[carla.LaneType](#carla.LaneType)_) - Limits the search for nearest lane to one or various lane types that can be flagged.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Map.get_waypoint_xodr"></a>**<font color="#7fb800">get_waypoint_xodr</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**road_id**</font>, <font color="#00a6ed">**lane_id**</font>, <font color="#00a6ed">**s**</font>)  
Returns a waypoint if all the parameters passed are correct. Otherwise, returns __None__.  
    - **Parameters:**
        - `road_id` (_int_) - ID of the road to get the waypoint.  
        - `lane_id` (_int_) - ID of the lane to get the waypoint.  
        - `s` (_float<small> - meters</small>_) - Specify the length from the road start.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  

##### Dunder methods
- <a name="carla.Map.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.MapLayer<a name="carla.MapLayer"></a>
Class that represents each manageable layer of the map. Can be used as flags. __WARNING: Only "Opt" maps are able to work with map layers.__.  

### Instance Variables
- <a name="carla.MapLayer.NONE"></a>**<font color="#f8805a">NONE</font>**  
No layers selected.  
- <a name="carla.MapLayer.Buildings"></a>**<font color="#f8805a">Buildings</font>**  
- <a name="carla.MapLayer.Decals"></a>**<font color="#f8805a">Decals</font>**  
- <a name="carla.MapLayer.Foliage"></a>**<font color="#f8805a">Foliage</font>**  
- <a name="carla.MapLayer.Ground"></a>**<font color="#f8805a">Ground</font>**  
- <a name="carla.MapLayer.ParkedVehicles"></a>**<font color="#f8805a">ParkedVehicles</font>**  
- <a name="carla.MapLayer.Particles"></a>**<font color="#f8805a">Particles</font>**  
- <a name="carla.MapLayer.Props"></a>**<font color="#f8805a">Props</font>**  
- <a name="carla.MapLayer.StreetLights"></a>**<font color="#f8805a">StreetLights</font>**  
- <a name="carla.MapLayer.Walls"></a>**<font color="#f8805a">Walls</font>**  
- <a name="carla.MapLayer.All"></a>**<font color="#f8805a">All</font>**  
All layers selected.  

---

## carla.MaterialParameter<a name="carla.MaterialParameter"></a>
Class that represents material parameters. Not all objects in the scene contain all parameters.  

### Instance Variables
- <a name="carla.MaterialParameter.Normal"></a>**<font color="#f8805a">Normal</font>**  
The Normal map of the object. Present in all objects.  
- <a name="carla.MaterialParameter.Diffuse"></a>**<font color="#f8805a">Diffuse</font>**  
The Diffuse texture of the object. Present in all objects.  
- <a name="carla.MaterialParameter.AO_Roughness_Metallic_Emissive"></a>**<font color="#f8805a">AO_Roughness_Metallic_Emissive</font>**  
A texture where each color channel represent a property of the material (R: Ambien oclusion, G: Roughness, B: Metallic, A: Emissive/Height map in some objects).  
- <a name="carla.MaterialParameter.Emissive"></a>**<font color="#f8805a">Emissive</font>**  
Emissive texture. Present in a few objects.  

---

## carla.ObstacleDetectionEvent<a name="carla.ObstacleDetectionEvent"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines the obstacle data for <b>sensor.other.obstacle</b>. Learn more about this [here](ref_sensors.md#obstacle-detector).  

### Instance Variables
- <a name="carla.ObstacleDetectionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
The actor the sensor is attached to.  
- <a name="carla.ObstacleDetectionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
The actor or object considered to be an obstacle.  
- <a name="carla.ObstacleDetectionEvent.distance"></a>**<font color="#f8805a">distance</font>** (_float<small> - meters</small>_)  
Distance between `actor` and `other`.  

### Methods

##### Dunder methods
- <a name="carla.ObstacleDetectionEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.OpendriveGenerationParameters<a name="carla.OpendriveGenerationParameters"></a>
This class defines the parameters used when generating a world using an OpenDRIVE file.  

### Instance Variables
- <a name="carla.OpendriveGenerationParameters.vertex_distance"></a>**<font color="#f8805a">vertex_distance</font>** (_float_)  
Distance between vertices of the mesh generated. __Default is `2.0`__.  
- <a name="carla.OpendriveGenerationParameters.max_road_length"></a>**<font color="#f8805a">max_road_length</font>** (_float_)  
Max road length for a single mesh portion. The mesh of the map is divided into portions, in order to avoid propagating issues. __Default is `50.0`__.  
- <a name="carla.OpendriveGenerationParameters.wall_height"></a>**<font color="#f8805a">wall_height</font>** (_float_)  
Height of walls created on the boundaries of the road. These prevent vehicles from falling off the road. __Default is `1.0`__.  
- <a name="carla.OpendriveGenerationParameters.additional_width"></a>**<font color="#f8805a">additional_width</font>** (_float_)  
Additional with applied junction lanes. Complex situations tend to occur at junctions, and a little increase can prevent vehicles from falling off the road.  __Default is `0.6`__.  
- <a name="carla.OpendriveGenerationParameters.smooth_junctions"></a>**<font color="#f8805a">smooth_junctions</font>** (_bool_)  
If __True__, the mesh at junctions will be smoothed to prevent issues where roads blocked other roads. __Default is `True`__.  
- <a name="carla.OpendriveGenerationParameters.enable_mesh_visibility"></a>**<font color="#f8805a">enable_mesh_visibility</font>** (_bool_)  
If __True__, the road mesh will be rendered. Setting this to __False__ should reduce the rendering overhead.  __Default is `True`__.  
- <a name="carla.OpendriveGenerationParameters.enable_pedestrian_navigation"></a>**<font color="#f8805a">enable_pedestrian_navigation</font>** (_bool_)  
If __True__, Pedestrian navigation will be enabled using Recast tool. For very large maps it is recomended to disable this option. __Default is `True`__.  

---

## carla.OpticalFlowImage<a name="carla.OpticalFlowImage"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines an optical flow image of 2-Dimension float (32-bit) vectors representing the optical flow detected in the field of view. The components of the vector represents the displacement of an object in the image plane. Each component outputs values in the normalized range [-2,2] which scales to [-2 size, 2 size] with size being the total resolution in the corresponding component.  

### Instance Variables
- <a name="carla.OpticalFlowImage.fov"></a>**<font color="#f8805a">fov</font>** (_float<small> - degrees</small>_)  
Horizontal field of view of the image.  
- <a name="carla.OpticalFlowImage.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Image height in pixels.  
- <a name="carla.OpticalFlowImage.width"></a>**<font color="#f8805a">width</font>** (_int_)  
Image width in pixels.  
- <a name="carla.OpticalFlowImage.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
Flattened array of pixel data, use reshape to create an image array.  

### Methods

##### Getters
- <a name="carla.OpticalFlowImage.get_color_coded_flow"></a>**<font color="#7fb800">get_color_coded_flow</font>**(<font color="#00a6ed">**self**</font>)  
Visualization helper. Converts the optical flow image to an RGB image.  
    - **Return:** _[carla.Image](#carla.Image)_  

##### Dunder methods
- <a name="carla.OpticalFlowImage.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
- <a name="carla.OpticalFlowImage.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.OpticalFlowPixel](#carla.OpticalFlowPixel) that form the image.  
- <a name="carla.OpticalFlowImage.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.OpticalFlowImage.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>, <font color="#00a6ed">**color**=[carla.Color](#carla.Color)</font>)  
- <a name="carla.OpticalFlowImage.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.OpticalFlowPixel<a name="carla.OpticalFlowPixel"></a>
Class that defines a 2 dimensional vector representing an optical flow pixel.  

### Instance Variables
- <a name="carla.OpticalFlowPixel.x"></a>**<font color="#f8805a">x</font>** (_float_)  
Optical flow in the x component.  
- <a name="carla.OpticalFlowPixel.y"></a>**<font color="#f8805a">y</font>** (_float_)  
Optical flow in the y component.  

### Methods
- <a name="carla.OpticalFlowPixel.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0</font>, <font color="#00a6ed">**y**=0</font>)  
Initializes the Optical Flow Pixel. Zero by default.  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  

##### Dunder methods
- <a name="carla.OpticalFlowPixel.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.OpticalFlowPixel](#carla.OpticalFlowPixel)</font>)  
- <a name="carla.OpticalFlowPixel.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.OpticalFlowPixel](#carla.OpticalFlowPixel)</font>)  
- <a name="carla.OpticalFlowPixel.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Osm2Odr<a name="carla.Osm2Odr"></a>
Class that converts an OpenStreetMap map to OpenDRIVE format, so that it can be loaded in CARLA. Find out more about this feature in the [docs](tuto_G_openstreetmap.md).  

### Methods
- <a name="carla.Osm2Odr.convert"></a>**<font color="#7fb800">convert</font>**(<font color="#00a6ed">**osm_file**</font>, <font color="#00a6ed">**settings**</font>)  
Takes the content of an <code>.osm</code> file (OpenStreetMap format) and returns the content of the <code>.xodr</code> (OpenDRIVE format) describing said map. Some parameterization is passed to do the conversion.  
    - **Parameters:**
        - `osm_file` (_str_) - The content of the input OpenStreetMap file parsed as string.  
        - `settings` (_[carla.OSM2ODRSettings](#carla.OSM2ODRSettings)_) - Parameterization for the conversion.  
    - **Return:** _str_  

---

## carla.Osm2OdrSettings<a name="carla.Osm2OdrSettings"></a>
Helper class that contains the parameterization that will be used by [carla.Osm2Odr](#carla.Osm2Odr) to convert an OpenStreetMap map to OpenDRIVE format. Find out more about this feature in the [docs](tuto_G_openstreetmap.md).  

### Instance Variables
- <a name="carla.Osm2OdrSettings.use_offsets"></a>**<font color="#f8805a">use_offsets</font>** (_bool_)  
Enables the use of offset for the conversion. The offset will move the origin position of the map. Default value is __False__.  
- <a name="carla.Osm2OdrSettings.offset_x"></a>**<font color="#f8805a">offset_x</font>** (_float<small> - meters</small>_)  
Offset in the X axis.  Default value is __0.0__.  
- <a name="carla.Osm2OdrSettings.offset_y"></a>**<font color="#f8805a">offset_y</font>** (_float<small> - meters</small>_)  
Offset in the Y axis.  Default value is __0.0__.  
- <a name="carla.Osm2OdrSettings.default_lane_width"></a>**<font color="#f8805a">default_lane_width</font>** (_float<small> - meters</small>_)  
Width of the lanes described in the resulting XODR map. Default value is __4.0__.  
- <a name="carla.Osm2OdrSettings.elevation_layer_height"></a>**<font color="#f8805a">elevation_layer_height</font>** (_float<small> - meters</small>_)  
Defines the height separating two different [OpenStreetMap layers](https://wiki.openstreetmap.org/wiki/Key:layer). Default value is __0.0__.  
- <a name="carla.Osm2OdrSettings.center_map"></a>**<font color="#f8805a">center_map</font>** (_bool_)  
When this option is enabled, the geometry of the map will be displaced so that the origin of coordinates matches the center of the bounding box of the entire road map.  
- <a name="carla.Osm2OdrSettings.proj_string"></a>**<font color="#f8805a">proj_string</font>** (_str_)  
Defines the [proj4](https://github.com/OSGeo/proj.4) string that will be used to compute the projection from geocoordinates to cartesian coordinates. This string will be written in the resulting OpenDRIVE unless the options `use_offsets` or `center_map` are enabled as these options override some of the definitions in the string.  
- <a name="carla.Osm2OdrSettings.generate_traffic_lights"></a>**<font color="#f8805a">generate_traffic_lights</font>** (_bool_)  
Indicates wether to generate traffic light data in the OpenDRIVE. Road types defined by `set_traffic_light_excluded_way_types(way_types)` will not generate traffic lights.  
- <a name="carla.Osm2OdrSettings.all_junctions_with_traffic_lights"></a>**<font color="#f8805a">all_junctions_with_traffic_lights</font>** (_bool_)  
When disabled, the converter will generate traffic light data from the OpenStreetMaps data only. When enabled, all junctions will generate traffic lights.  

### Methods

##### Setters
- <a name="carla.Osm2OdrSettings.set_osm_way_types"></a>**<font color="#7fb800">set_osm_way_types</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**way_types**</font>)  
Defines the OpenStreetMaps road types that will be imported to OpenDRIVE. By default the road types imported are `motorway, motorway_link, trunk, trunk_link, primary, primary_link, secondary, secondary_link, tertiary, tertiary_link, unclassified, residential`. For a full list of road types check [here](https://wiki.openstreetmap.org/wiki/Main_Page).  
    - **Parameters:**
        - `way_types` (_list(str)_) - The list of road types.  
- <a name="carla.Osm2OdrSettings.set_traffic_light_excluded_way_types"></a>**<font color="#7fb800">set_traffic_light_excluded_way_types</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**way_types**</font>)  
Defines the OpenStreetMaps road types that will not generate traffic lights even if `generate_traffic_lights` is enabled. By default the road types excluded are `motorway_link, primary_link, secondary_link, tertiary_link`.  
    - **Parameters:**
        - `way_types` (_list(str)_) - The list of road types.  

---

## carla.RadarDetection<a name="carla.RadarDetection"></a>
Data contained inside a [carla.RadarMeasurement](#carla.RadarMeasurement). Each of these represents one of the points in the cloud that a <b>sensor.other.radar</b> registers and contains the distance, angle and velocity in relation to the radar.  

### Instance Variables
- <a name="carla.RadarDetection.altitude"></a>**<font color="#f8805a">altitude</font>** (_float<small> - radians</small>_)  
Altitude angle of the detection.  
- <a name="carla.RadarDetection.azimuth"></a>**<font color="#f8805a">azimuth</font>** (_float<small> - radians</small>_)  
Azimuth angle of the detection.  
- <a name="carla.RadarDetection.depth"></a>**<font color="#f8805a">depth</font>** (_float<small> - meters</small>_)  
Distance from the sensor to the detection position.  
- <a name="carla.RadarDetection.velocity"></a>**<font color="#f8805a">velocity</font>** (_float<small> - m/s</small>_)  
The velocity of the detected object towards the sensor.  

### Methods

##### Dunder methods
- <a name="carla.RadarDetection.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RadarMeasurement<a name="carla.RadarMeasurement"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines and gathers the measures registered by a <b>sensor.other.radar</b>, representing a wall of points in front of the sensor with a distance, angle and velocity in relation to it. The data consists of a [carla.RadarDetection](#carla.RadarDetection) array. Learn more about this [here](ref_sensors.md#radar-sensor).  

### Instance Variables
- <a name="carla.RadarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
The complete information of the [carla.RadarDetection](#carla.RadarDetection) the radar has registered.  

### Methods

##### Getters
- <a name="carla.RadarMeasurement.get_detection_count"></a>**<font color="#7fb800">get_detection_count</font>**(<font color="#00a6ed">**self**</font>)  
Retrieves the number of entries generated, same as **<font color="#7fb800">\__str__()</font>**.  

##### Dunder methods
- <a name="carla.RadarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
- <a name="carla.RadarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.RadarDetection](#carla.RadarDetection) retrieved as data.  
- <a name="carla.RadarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.RadarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>, <font color="#00a6ed">**detection**=[carla.RadarDetection](#carla.RadarDetection)</font>)  
- <a name="carla.RadarMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Rotation<a name="carla.Rotation"></a>
Class that represents a 3D rotation and therefore, an orientation in space. CARLA uses the Unreal Engine coordinates system. This is a Z-up left-handed system.  <br>
<br>The constructor method follows a specific order of declaration: `(pitch, yaw, roll)`, which corresponds to `(Y-rotation,Z-rotation,X-rotation)`.  <br> <br>![UE4_Rotation](https://d26ilriwvtzlb.cloudfront.net/8/83/BRMC_9.jpg) *Unreal Engine's coordinates system*.  

### Instance Variables
- <a name="carla.Rotation.pitch"></a>**<font color="#f8805a">pitch</font>** (_float<small> - degrees</small>_)  
Y-axis rotation angle.  
- <a name="carla.Rotation.yaw"></a>**<font color="#f8805a">yaw</font>** (_float<small> - degrees</small>_)  
Z-axis rotation angle.  
- <a name="carla.Rotation.roll"></a>**<font color="#f8805a">roll</font>** (_float<small> - degrees</small>_)  
X-axis rotation angle.  

### Methods
- <a name="carla.Rotation.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pitch**=0.0</font>, <font color="#00a6ed">**yaw**=0.0</font>, <font color="#00a6ed">**roll**=0.0</font>)  
    - **Parameters:**
        - `pitch` (_float<small> - degrees</small>_) - Y-axis rotation angle.  
        - `yaw` (_float<small> - degrees</small>_) - Z-axis rotation angle.  
        - `roll` (_float<small> - degrees</small>_) - X-axis rotation angle.  
    - **Warning:** <font color="#ED2F2F">_The declaration order is different in CARLA <code>(pitch,yaw,roll)</code>, and in the Unreal Engine Editor <code>(roll,pitch,yaw)</code>. When working in a build from source, don't mix up the axes' rotations._</font>  

##### Getters
- <a name="carla.Rotation.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes the vector pointing forward according to the rotation of the object.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Rotation.get_right_vector"></a>**<font color="#7fb800">get_right_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes the vector pointing to the right according to the rotation of the object.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Rotation.get_up_vector"></a>**<font color="#7fb800">get_up_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes the vector pointing upwards according to the rotation of the object.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  

##### Dunder methods
- <a name="carla.Rotation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Rotation](#carla.Rotation)</font>)  
Returns __True__ if both rotations represent the same orientation for every axis.  
    - **Return:** _bool_  
- <a name="carla.Rotation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Rotation](#carla.Rotation)</font>)  
Returns __True__ if both rotations represent the same orientation for every axis.  
    - **Return:** _bool_  
- <a name="carla.Rotation.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses the axis' orientations to string.  

---

## carla.RssActorConstellationData<a name="carla.RssActorConstellationData"></a>
Data structure that is provided within the callback registered by RssSensor.register_actor_constellation_callback().  

### Instance Variables
- <a name="carla.RssActorConstellationData.ego_match_object"></a>**<font color="#f8805a">ego_match_object</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/structad_1_1map_1_1match_1_1Object.html">ad.map.match.Object</a>_)  
The ego map matched information.  
- <a name="carla.RssActorConstellationData.ego_route"></a>**<font color="#f8805a">ego_route</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/structad_1_1map_1_1route_1_1FullRoute.html">ad.map.route.FullRoute</a>_)  
The ego route.  
- <a name="carla.RssActorConstellationData.ego_dynamics_on_route"></a>**<font color="#f8805a">ego_dynamics_on_route</font>** (_[carla.RssEgoDynamicsOnRoute](#carla.RssEgoDynamicsOnRoute)_)  
Current ego vehicle dynamics regarding the route.  
- <a name="carla.RssActorConstellationData.other_match_object"></a>**<font color="#f8805a">other_match_object</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/structad_1_1map_1_1match_1_1Object.html">ad.map.match.Object</a>_)  
The other object's map matched information. This is only valid if 'other_actor' is not 'None'.  
- <a name="carla.RssActorConstellationData.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
The other actor. This is 'None' in case of query of default parameters or articial objects of kind <a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/namespacead_1_1rss_1_1world.html#a6432f1ef8d0657b4f21ed5966aca1625">ad.rss.world.ObjectType.ArtificialObject</a> with no dedicated '[carla.Actor](#carla.Actor)' (as e.g. for the [road boundaries](ref_sensors.md#rss-sensor) at the moment).  

### Methods

##### Dunder methods
- <a name="carla.RssActorConstellationData.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RssActorConstellationResult<a name="carla.RssActorConstellationResult"></a>
Data structure that should be returned by the callback registered by RssSensor.register_actor_constellation_callback().  

### Instance Variables
- <a name="carla.RssActorConstellationResult.rss_calculation_mode"></a>**<font color="#f8805a">rss_calculation_mode</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss_map_integration/namespacead_1_1rss_1_1map.html#adcb01232986ed83a0c540cd5d03ef495">ad.rss.map.RssMode</a>_)  
The calculation mode to be applied with the actor.  
- <a name="carla.RssActorConstellationResult.restrict_speed_limit_mode"></a>**<font color="#f8805a">restrict_speed_limit_mode</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss_map_integration/classad_1_1rss_1_1map_1_1RssSceneCreation.html#a403aae6dce3c77a8aec01dd9808dd964">ad.rss.map.RestrictSpeedLimitMode</a>_)  
The mode for restricting speed limit.  
- <a name="carla.RssActorConstellationResult.ego_vehicle_dynamics"></a>**<font color="#f8805a">ego_vehicle_dynamics</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1RssDynamics.html">ad.rss.world.RssDynamics</a>_)  
The RSS dynamics to be applied for the ego vehicle.  
- <a name="carla.RssActorConstellationResult.actor_object_type"></a>**<font color="#f8805a">actor_object_type</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/namespacead_1_1rss_1_1world.html#a6432f1ef8d0657b4f21ed5966aca1625">ad.rss.world.ObjectType</a>_)  
The RSS object type to be used for the actor.  
- <a name="carla.RssActorConstellationResult.actor_dynamics"></a>**<font color="#f8805a">actor_dynamics</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1RssDynamics.html">ad.rss.world.RssDynamics</a>_)  
The RSS dynamics to be applied for the actor.  

### Methods

##### Dunder methods
- <a name="carla.RssActorConstellationResult.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RssEgoDynamicsOnRoute<a name="carla.RssEgoDynamicsOnRoute"></a>
Part of the data contained inside a [carla.RssResponse](#carla.RssResponse) describing the state of the vehicle. The parameters include its current dynamics, and how it is heading regarding the target route.  

### Instance Variables
- <a name="carla.RssEgoDynamicsOnRoute.ego_speed"></a>**<font color="#f8805a">ego_speed</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Speed.html">ad.physics.Speed</a>_)  
The ego vehicle's speed.  
- <a name="carla.RssEgoDynamicsOnRoute.min_stopping_distance"></a>**<font color="#f8805a">min_stopping_distance</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Distance.html">ad.physics.Distance</a>_)  
The current minimum stopping distance.  
- <a name="carla.RssEgoDynamicsOnRoute.ego_center"></a>**<font color="#f8805a">ego_center</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/structad_1_1map_1_1point_1_1ENUPoint.html">ad.map.point.ENUPoint</a>_)  
The considered enu position of the ego vehicle.  
- <a name="carla.RssEgoDynamicsOnRoute.ego_heading"></a>**<font color="#f8805a">ego_heading</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/classad_1_1map_1_1point_1_1ENUHeading.html">ad.map.point.ENUHeading</a>_)  
The considered heading of the ego vehicle.  
- <a name="carla.RssEgoDynamicsOnRoute.ego_center_within_route"></a>**<font color="#f8805a">ego_center_within_route</font>** (_bool_)  
States if the ego vehicle's center is within the route.  
- <a name="carla.RssEgoDynamicsOnRoute.crossing_border"></a>**<font color="#f8805a">crossing_border</font>** (_bool_)  
States if the vehicle is already crossing one of the lane borders.  
- <a name="carla.RssEgoDynamicsOnRoute.route_heading"></a>**<font color="#f8805a">route_heading</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/classad_1_1map_1_1point_1_1ENUHeading.html">ad.map.point.ENUHeading</a>_)  
The considered heading of the route.  
- <a name="carla.RssEgoDynamicsOnRoute.route_nominal_center"></a>**<font color="#f8805a">route_nominal_center</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/structad_1_1map_1_1point_1_1ENUPoint.html">ad.map.point.ENUPoint</a>_)  
The considered nominal center of the current route.  
- <a name="carla.RssEgoDynamicsOnRoute.heading_diff"></a>**<font color="#f8805a">heading_diff</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_map_access/apidoc/html/classad_1_1map_1_1point_1_1ENUHeading.html">ad.map.point.ENUHeading</a>_)  
The considered heading diff towards the route.  
- <a name="carla.RssEgoDynamicsOnRoute.route_speed_lat"></a>**<font color="#f8805a">route_speed_lat</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Speed.html">ad.physics.Speed</a>_)  
The ego vehicle's speed component _lat_ regarding the route.  
- <a name="carla.RssEgoDynamicsOnRoute.route_speed_lon"></a>**<font color="#f8805a">route_speed_lon</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Speed.html">ad.physics.Speed</a>_)  
The ego vehicle's speed component _lon_ regarding the route.  
- <a name="carla.RssEgoDynamicsOnRoute.route_accel_lat"></a>**<font color="#f8805a">route_accel_lat</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Acceleration.html">ad.physics.Acceleration</a>_)  
The ego vehicle's acceleration component _lat_ regarding the route.  
- <a name="carla.RssEgoDynamicsOnRoute.route_accel_lon"></a>**<font color="#f8805a">route_accel_lon</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Acceleration.html">ad.physics.Acceleration</a>_)  
The ego vehicle's acceleration component _lon_ regarding the route.  
- <a name="carla.RssEgoDynamicsOnRoute.avg_route_accel_lat"></a>**<font color="#f8805a">avg_route_accel_lat</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Acceleration.html">ad.physics.Acceleration</a>_)  
The ego vehicle's acceleration component _lat_ regarding the route smoothened by an average filter.  
- <a name="carla.RssEgoDynamicsOnRoute.avg_route_accel_lon"></a>**<font color="#f8805a">avg_route_accel_lon</font>** (_<a href="https://ad-map-access.readthedocs.io/en/latest/ad_physics/apidoc/html/classad_1_1physics_1_1Acceleration.html">ad.physics.Acceleration</a>_)  
The ego acceleration component _lon_ regarding the route smoothened by an average filter.  

### Methods

##### Dunder methods
- <a name="carla.RssEgoDynamicsOnRoute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RssLogLevel<a name="carla.RssLogLevel"></a>
Enum declaration used in [carla.RssSensor](#carla.RssSensor) to set the log level.  

### Instance Variables
- <a name="carla.RssLogLevel.trace"></a>**<font color="#f8805a">trace</font>**  
- <a name="carla.RssLogLevel.debug"></a>**<font color="#f8805a">debug</font>**  
- <a name="carla.RssLogLevel.info"></a>**<font color="#f8805a">info</font>**  
- <a name="carla.RssLogLevel.warn"></a>**<font color="#f8805a">warn</font>**  
- <a name="carla.RssLogLevel.err"></a>**<font color="#f8805a">err</font>**  
- <a name="carla.RssLogLevel.critical"></a>**<font color="#f8805a">critical</font>**  
- <a name="carla.RssLogLevel.off"></a>**<font color="#f8805a">off</font>**  

---

## carla.RssResponse<a name="carla.RssResponse"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that contains the output of a [carla.RssSensor](#carla.RssSensor). This is the result of the RSS calculations performed for the parent vehicle of the sensor.

A [carla.RssRestrictor](#carla.RssRestrictor) will use the data to modify the [carla.VehicleControl](#carla.VehicleControl) of the vehicle.  

### Instance Variables
- <a name="carla.RssResponse.response_valid"></a>**<font color="#f8805a">response_valid</font>** (_bool_)  
States if the response is valid. It is __False__ if calculations failed or an exception occured.  
- <a name="carla.RssResponse.proper_response"></a>**<font color="#f8805a">proper_response</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1ProperResponse.html">ad.rss.state.ProperResponse</a>_)  
The proper response that the RSS calculated for the vehicle.  
- <a name="carla.RssResponse.rss_state_snapshot"></a>**<font color="#f8805a">rss_state_snapshot</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1RssStateSnapshot.html">ad.rss.state.RssStateSnapshot</a>_)  
Detailed RSS states at the current moment in time.  
- <a name="carla.RssResponse.ego_dynamics_on_route"></a>**<font color="#f8805a">ego_dynamics_on_route</font>** (_[carla.RssEgoDynamicsOnRoute](#carla.RssEgoDynamicsOnRoute)_)  
Current ego vehicle dynamics regarding the route.  
- <a name="carla.RssResponse.world_model"></a>**<font color="#f8805a">world_model</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1WorldModel.html">ad.rss.world.WorldModel</a>_)  
World model used for calculations.  
- <a name="carla.RssResponse.situation_snapshot"></a>**<font color="#f8805a">situation_snapshot</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1situation_1_1SituationSnapshot.html">ad.rss.situation.SituationSnapshot</a>_)  
Detailed RSS situations extracted from the world model.  

### Methods

##### Dunder methods
- <a name="carla.RssResponse.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RssRestrictor<a name="carla.RssRestrictor"></a>
These objects apply restrictions to a [carla.VehicleControl](#carla.VehicleControl). It is part of the CARLA implementation of the [C++ Library for Responsibility Sensitive Safety](https://github.com/intel/ad-rss-lib). This class works hand in hand with a [rss sensor](ref_sensors.md#rss-sensor), which provides the data of the restrictions to be applied.  

### Methods
- <a name="carla.RssRestrictor.restrict_vehicle_control"></a>**<font color="#7fb800">restrict_vehicle_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vehicle_control**</font>, <font color="#00a6ed">**proper_response**</font>, <font color="#00a6ed">**ego_dynamics_on_route**</font>, <font color="#00a6ed">**vehicle_physics**</font>)  
Applies the safety restrictions given by a [carla.RssSensor](#carla.RssSensor) to a [carla.VehicleControl](#carla.VehicleControl).  
    - **Parameters:**
        - `vehicle_control` (_[carla.VehicleControl](#carla.VehicleControl)_) - The input vehicle control to be restricted.  
        - `proper_response` (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1ProperResponse.html">ad.rss.state.ProperResponse</a>_) - Part of the response generated by the sensor. Contains restrictions to be applied to the acceleration of the vehicle.  
        - `ego_dynamics_on_route` (_[carla.RssEgoDynamicsOnRoute](#carla.RssEgoDynamicsOnRoute)_) - Part of the response generated by the sensor. Contains dynamics and heading of the vehicle regarding its route.  
        - `vehicle_physics` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_) - The current physics of the vehicle. Used to apply the restrictions properly.  
    - **Return:** _[carla.VehicleControl](#carla.VehicleControl)_  

##### Setters
- <a name="carla.RssRestrictor.set_log_level"></a>**<font color="#7fb800">set_log_level</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**log_level**</font>)  
Sets the log level.  
    - **Parameters:**
        - `log_level` (_[carla.RssLogLevel](#carla.RssLogLevel)_) - New log level.  

---

## carla.RssRoadBoundariesMode<a name="carla.RssRoadBoundariesMode"></a>
Enum declaration used in [carla.RssSensor](#carla.RssSensor) to enable or disable the [stay on road](https://intel.github.io/ad-rss-lib/ad_rss_map_integration/HandleRoadBoundaries/) feature. In summary, this feature considers the road boundaries as virtual objects. The minimum safety distance check is applied to these virtual walls, in order to make sure the vehicle does not drive off the road.  

### Instance Variables
- <a name="carla.RssRoadBoundariesMode.On"></a>**<font color="#f8805a">On</font>**  
Enables the _stay on road_ feature.  
- <a name="carla.RssRoadBoundariesMode.Off"></a>**<font color="#f8805a">Off</font>**  
Disables the _stay on road_ feature.  

---

## carla.RssSensor<a name="carla.RssSensor"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Sensor](#carla.Sensor)_</small></br>
This sensor works a bit differently than the rest. Take look at the [specific documentation](adv_rss.md), and the [rss sensor reference](ref_sensors.md#rss-sensor) to gain full understanding of it.

The RSS sensor uses world information, and a [RSS library](https://github.com/intel/ad-rss-lib) to make safety checks on a vehicle. The output retrieved by the sensor is a [carla.RssResponse](#carla.RssResponse). This will be used by a [carla.RssRestrictor](#carla.RssRestrictor) to modify a [carla.VehicleControl](#carla.VehicleControl) before applying it to a vehicle.  

### Instance Variables
- <a name="carla.RssSensor.ego_vehicle_dynamics"></a>**<font color="#f8805a">ego_vehicle_dynamics</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1RssDynamics.html">ad.rss.world.RssDynamics</a>_)  
States the [RSS parameters](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/) that the sensor will consider for the ego vehicle if no actor constellation callback is registered.  
- <a name="carla.RssSensor.other_vehicle_dynamics"></a>**<font color="#f8805a">other_vehicle_dynamics</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1RssDynamics.html">ad.rss.world.RssDynamics</a>_)  
States the [RSS parameters](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/) that the sensor will consider for the rest of vehicles if no actor constellation callback is registered.  
- <a name="carla.RssSensor.pedestrian_dynamics"></a>**<font color="#f8805a">pedestrian_dynamics</font>** (_<a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1RssDynamics.html">ad.rss.world.RssDynamics</a>_)  
States the [RSS parameters](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/) that the sensor will consider for pedestrians if no actor constellation callback is registered.  
- <a name="carla.RssSensor.road_boundaries_mode"></a>**<font color="#f8805a">road_boundaries_mode</font>** (_[carla.RssRoadBoundariesMode](#carla.RssRoadBoundariesMode)_)  
Switches the [stay on road](https://intel.github.io/ad-rss-lib/ad_rss_map_integration/HandleRoadBoundaries/) feature. By default is __Off__.  
- <a name="carla.RssSensor.routing_targets"></a>**<font color="#f8805a">routing_targets</font>** (_vector<[carla.Transform](#carla.Transform)>_)  
The current list of targets considered to route the vehicle. If no routing targets are defined, a route is generated at random.  

### Methods
- <a name="carla.RssSensor.append_routing_target"></a>**<font color="#7fb800">append_routing_target</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**routing_target**</font>)  
Appends a new target position to the current route of the vehicle.  
    - **Parameters:**
        - `routing_target` (_[carla.Transform](#carla.Transform)_) - New target point for the route. Choose these after the intersections to force the route to take the desired turn.  
- <a name="carla.RssSensor.drop_route"></a>**<font color="#7fb800">drop_route</font>**(<font color="#00a6ed">**self**</font>)  
Discards the current route. If there are targets remaining in **<font color="#f8805a">routing_targets</font>**, creates a new route using those. Otherwise, a new route is created at random.  
- <a name="carla.RssSensor.register_actor_constellation_callback"></a>**<font color="#7fb800">register_actor_constellation_callback</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)  
Register a callback to customize a [carla.RssActorConstellationResult](#carla.RssActorConstellationResult). By this callback the settings of RSS parameters are done per actor constellation and the settings (ego_vehicle_dynamics, other_vehicle_dynamics and pedestrian_dynamics) have no effect.  
    - **Parameters:**
        - `callback` - The function to be called whenever a RSS situation is about to be calculated.  
- <a name="carla.RssSensor.reset_routing_targets"></a>**<font color="#7fb800">reset_routing_targets</font>**(<font color="#00a6ed">**self**</font>)  
Erases the targets that have been appended to the route.  

##### Setters
- <a name="carla.RssSensor.set_log_level"></a>**<font color="#7fb800">set_log_level</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**log_level**</font>)  
Sets the log level.  
    - **Parameters:**
        - `log_level` (_[carla.RssLogLevel](#carla.RssLogLevel)_) - New log level.  
- <a name="carla.RssSensor.set_map_log_level"></a>**<font color="#7fb800">set_map_log_level</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**log_level**</font>)  
Sets the map log level.  
    - **Parameters:**
        - `log_level` (_[carla.RssLogLevel](#carla.RssLogLevel)_) - New map log level.  

##### Dunder methods
- <a name="carla.RssSensor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.SemanticLidarDetection<a name="carla.SemanticLidarDetection"></a>
Data contained inside a [carla.SemanticLidarMeasurement](#carla.SemanticLidarMeasurement). Each of these represents one of the points in the cloud with its location, the cosine of the incident angle, index of the object hit, and its semantic tag.  

### Instance Variables
- <a name="carla.SemanticLidarDetection.point"></a>**<font color="#f8805a">point</font>** (_[carla.Location](#carla.Location)<small> - meters</small>_)  
[x,y,z] coordinates of the point.  
- <a name="carla.SemanticLidarDetection.cos_inc_angle"></a>**<font color="#f8805a">cos_inc_angle</font>** (_float_)  
Cosine of the incident angle between the ray, and the normal of the hit object.  
- <a name="carla.SemanticLidarDetection.object_idx"></a>**<font color="#f8805a">object_idx</font>** (_uint_)  
ID of the actor hit by the ray.  
- <a name="carla.SemanticLidarDetection.object_tag"></a>**<font color="#f8805a">object_tag</font>** (_uint_)  
[Semantic tag](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/ref_sensors/#semantic-segmentation-camera) of the component hit by the ray.  

### Methods

##### Dunder methods
- <a name="carla.SemanticLidarDetection.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.SemanticLidarMeasurement<a name="carla.SemanticLidarMeasurement"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.SensorData](#carla.SensorData)_</small></br>
Class that defines the semantic LIDAR data retrieved by a <b>sensor.lidar.ray_cast_semantic</b>. This essentially simulates a rotating LIDAR using ray-casting. Learn more about this [here](ref_sensors.md#semanticlidar-raycast-sensor).  

### Instance Variables
- <a name="carla.SemanticLidarMeasurement.channels"></a>**<font color="#f8805a">channels</font>** (_int_)  
Number of lasers shot.  
- <a name="carla.SemanticLidarMeasurement.horizontal_angle"></a>**<font color="#f8805a">horizontal_angle</font>** (_float<small> - radians</small>_)  
Horizontal angle the LIDAR is rotated at the time of the measurement.  
- <a name="carla.SemanticLidarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
Received list of raw detection points. Each point consists of [x,y,z] coordinates plus the cosine of the incident angle, the index of the hit actor, and its semantic tag.  

### Methods
- <a name="carla.SemanticLidarMeasurement.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Saves the point cloud to disk as a <b>.ply</b> file describing data from 3D scanners. The files generated are ready to be used within [MeshLab](http://www.meshlab.net/), an open-source system for processing said files. Just take into account that axis may differ from Unreal Engine and so, need to be reallocated.  
    - **Parameters:**
        - `path` (_str_)  

##### Getters
- <a name="carla.SemanticLidarMeasurement.get_point_count"></a>**<font color="#7fb800">get_point_count</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**channel**</font>)  
Retrieves the number of points sorted by channel that are generated by this measure. Sorting by channel allows to identify the original channel for every point.  
    - **Parameters:**
        - `channel` (_int_)  

##### Dunder methods
- <a name="carla.SemanticLidarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>)  
- <a name="carla.SemanticLidarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.SemanticLidarDetection](#carla.SemanticLidarDetection) retrieved as data.  
- <a name="carla.SemanticLidarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.SemanticLidarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**=int</font>, <font color="#00a6ed">**detection**=[carla.SemanticLidarDetection](#carla.SemanticLidarDetection)</font>)  
- <a name="carla.SemanticLidarMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Sensor<a name="carla.Sensor"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Actor](#carla.Actor)_</small></br>
Sensors compound a specific family of actors quite diverse and unique. They are normally spawned as attachment/sons of a vehicle (take a look at [carla.World](#carla.World) to learn about actor spawning). Sensors are thoroughly designed to retrieve different types of data that they are listening to. The data they receive is shaped as different subclasses inherited from [carla.SensorData](#carla.SensorData) (depending on the sensor).

  Most sensors can be divided in two groups: those receiving data on every tick (cameras, point clouds and some specific sensors) and those who only receive under certain circumstances (trigger detectors). CARLA provides a specific set of sensors and their blueprint can be found in [carla.BlueprintLibrary](#carla.BlueprintLibrary). All the information on their preferences and settlement can be found [here](ref_sensors.md), but the list of those available in CARLA so far goes as follow.
  <br><b>Receive data on every tick.</b>
  - [Depth camera](ref_sensors.md#depth-camera).
  - [Gnss sensor](ref_sensors.md#gnss-sensor).
  - [IMU sensor](ref_sensors.md#imu-sensor).
  - [Lidar raycast](ref_sensors.md#lidar-raycast-sensor).
  - [SemanticLidar raycast](ref_sensors.md#semanticlidar-raycast-sensor).
  - [Radar](ref_sensors.md#radar-sensor).
  - [RGB camera](ref_sensors.md#rgb-camera).
  - [RSS sensor](ref_sensors.md#rss-sensor).
  - [Semantic Segmentation camera](ref_sensors.md#semantic-segmentation-camera).
  <br><b>Only receive data when triggered.</b>
  - [Collision detector](ref_sensors.md#collision-detector).
  - [Lane invasion detector](ref_sensors.md#lane-invasion-detector).
  - [Obstacle detector](ref_sensors.md#obstacle-detector).  

### Instance Variables
- <a name="carla.Sensor.is_listening"></a>**<font color="#f8805a">is_listening</font>** (_boolean_)  
When <b>True</b> the sensor will be waiting for data.  

### Methods
- <a name="carla.Sensor.is_listening"></a>**<font color="#7fb800">is_listening</font>**(<font color="#00a6ed">**self**</font>)  
Returns whether the sensor is in a listening state.  
- <a name="carla.Sensor.is_listening_gbuffer"></a>**<font color="#7fb800">is_listening_gbuffer</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**gbuffer_id**</font>)  
Returns whether the sensor is in a listening state for a specific GBuffer texture.  
    - **Parameters:**
        - `gbuffer_id` (_[carla.GBufferTextureID](#carla.GBufferTextureID)_) - The ID of the target Unreal Engine GBuffer texture.  
- <a name="carla.Sensor.listen"></a>**<font color="#7fb800">listen</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)<button class="SnipetButton" id="carla.Sensor.listen-snipet_button">snippet &rarr;</button>  
The function the sensor will be calling to every time a new measurement is received. This function needs for an argument containing an object type [carla.SensorData](#carla.SensorData) to work with.  
    - **Parameters:**
        - `callback` (_function_) - The called function with one argument containing the sensor data.  
- <a name="carla.Sensor.listen_to_gbuffer"></a>**<font color="#7fb800">listen_to_gbuffer</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**gbuffer_id**</font>, <font color="#00a6ed">**callback**</font>)  
The function the sensor will be calling to every time the desired GBuffer texture is received.<br> This function needs for an argument containing an object type [carla.SensorData](#carla.SensorData) to work with.  
    - **Parameters:**
        - `gbuffer_id` (_[carla.GBufferTextureID](#carla.GBufferTextureID)_) - The ID of the target Unreal Engine GBuffer texture.  
        - `callback` (_function_) - The called function with one argument containing the received GBuffer texture.  
- <a name="carla.Sensor.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)  
Commands the sensor to stop listening for data.  
- <a name="carla.Sensor.stop_gbuffer"></a>**<font color="#7fb800">stop_gbuffer</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**gbuffer_id**</font>)  
Commands the sensor to stop listening for the specified GBuffer texture.  
    - **Parameters:**
        - `gbuffer_id` (_[carla.GBufferTextureID](#carla.GBufferTextureID)_) - The ID of the Unreal Engine GBuffer texture.  

##### Dunder methods
- <a name="carla.Sensor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.SensorData<a name="carla.SensorData"></a>
Base class for all the objects containing data generated by a [carla.Sensor](#carla.Sensor). This objects should be the argument of the function said sensor is listening to, in order to work with them. Each of these sensors needs for a specific type of sensor data. Hereunder is a list of the sensors and their corresponding data.<br>
  - Cameras (RGB, depth and semantic segmentation): [carla.Image](#carla.Image).<br>
  - Collision detector: [carla.CollisionEvent](#carla.CollisionEvent).<br>
  - GNSS sensor: [carla.GnssMeasurement](#carla.GnssMeasurement).<br>
  - IMU sensor: [carla.IMUMeasurement](#carla.IMUMeasurement).<br>
  - Lane invasion detector: [carla.LaneInvasionEvent](#carla.LaneInvasionEvent).<br>
  - LIDAR sensor: [carla.LidarMeasurement](#carla.LidarMeasurement).<br>
  - Obstacle detector: [carla.ObstacleDetectionEvent](#carla.ObstacleDetectionEvent).<br>
  - Radar sensor: [carla.RadarMeasurement](#carla.RadarMeasurement).<br>
  - RSS sensor: [carla.RssResponse](#carla.RssResponse).<br>
  - Semantic LIDAR sensor: [carla.SemanticLidarMeasurement](#carla.SemanticLidarMeasurement).  

### Instance Variables
- <a name="carla.SensorData.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
Frame count when the data was generated.  
- <a name="carla.SensorData.timestamp"></a>**<font color="#f8805a">timestamp</font>** (_float<small> - seconds</small>_)  
Simulation-time when the data was generated.  
- <a name="carla.SensorData.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Sensor's transform when the data was generated.  

---

## carla.TextureColor<a name="carla.TextureColor"></a>
Class representing a texture object to be uploaded to the server. Pixel format is RGBA, uint8 per channel.  

### Instance Variables
- <a name="carla.TextureColor.width"></a>**<font color="#f8805a">width</font>** (_int_)  
X-coordinate size of the texture.  
- <a name="carla.TextureColor.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Y-coordinate size of the texture.  

### Methods
- <a name="carla.TextureColor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**width**</font>, <font color="#00a6ed">**height**</font>)  
Initializes a the texture with a (`width`, `height`) size.  
    - **Parameters:**
        - `width` (_int_)  
        - `height` (_int_)  
- <a name="carla.TextureColor.get"></a>**<font color="#7fb800">get</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**</font>, <font color="#00a6ed">**y**</font>)  
Get the (x,y) pixel data.  
    - **Parameters:**
        - `x` (_int_)  
        - `y` (_int_)  
    - **Return:** _[carla.Color](#carla.Color)_  
- <a name="carla.TextureColor.set"></a>**<font color="#7fb800">set</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**</font>, <font color="#00a6ed">**y**</font>, <font color="#00a6ed">**value**</font>)  
Sets the (x,y) pixel data with `value`.  
    - **Parameters:**
        - `x` (_int_)  
        - `y` (_int_)  
        - `value` (_[carla.Color](#carla.Color)_)  

##### Setters
- <a name="carla.TextureColor.set_dimensions"></a>**<font color="#7fb800">set_dimensions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**width**</font>, <font color="#00a6ed">**height**</font>)  
Resizes the texture to te specified dimensions.  
    - **Parameters:**
        - `width` (_int_)  
        - `height` (_int_)  

---

## carla.TextureFloatColor<a name="carla.TextureFloatColor"></a>
Class representing a texture object to be uploaded to the server. Pixel format is RGBA, float per channel.  

### Instance Variables
- <a name="carla.TextureFloatColor.width"></a>**<font color="#f8805a">width</font>** (_int_)  
X-coordinate size of the texture.  
- <a name="carla.TextureFloatColor.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Y-coordinate size of the texture.  

### Methods
- <a name="carla.TextureFloatColor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**width**</font>, <font color="#00a6ed">**height**</font>)  
Initializes a the texture with a (`width`, `height`) size.  
    - **Parameters:**
        - `width` (_int_)  
        - `height` (_int_)  
- <a name="carla.TextureFloatColor.get"></a>**<font color="#7fb800">get</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**</font>, <font color="#00a6ed">**y**</font>)  
Get the (x,y) pixel data.  
    - **Parameters:**
        - `x` (_int_)  
        - `y` (_int_)  
    - **Return:** _[carla.FloatColor](#carla.FloatColor)_  
- <a name="carla.TextureFloatColor.set"></a>**<font color="#7fb800">set</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**</font>, <font color="#00a6ed">**y**</font>, <font color="#00a6ed">**value**</font>)  
Sets the (x,y) pixel data with `value`.  
    - **Parameters:**
        - `x` (_int_)  
        - `y` (_int_)  
        - `value` (_[carla.FloatColor](#carla.FloatColor)_)  

##### Setters
- <a name="carla.TextureFloatColor.set_dimensions"></a>**<font color="#7fb800">set_dimensions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**width**</font>, <font color="#00a6ed">**height**</font>)  
Resizes the texture to te specified dimensions.  
    - **Parameters:**
        - `width` (_int_)  
        - `height` (_int_)  

---

## carla.Timestamp<a name="carla.Timestamp"></a>
Class that contains time information for simulated data. This information is automatically retrieved as part of the [carla.WorldSnapshot](#carla.WorldSnapshot) the client gets on every frame, but might also be used in many other situations such as a [carla.Sensor](#carla.Sensor) retrieveing data.  

### Instance Variables
- <a name="carla.Timestamp.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
The number of frames elapsed since the simulator was launched.  
- <a name="carla.Timestamp.elapsed_seconds"></a>**<font color="#f8805a">elapsed_seconds</font>** (_float<small> - seconds</small>_)  
Simulated seconds elapsed since the beginning of the current episode.  
- <a name="carla.Timestamp.delta_seconds"></a>**<font color="#f8805a">delta_seconds</font>** (_float<small> - seconds</small>_)  
Simulated seconds elapsed since the previous frame.  
- <a name="carla.Timestamp.platform_timestamp"></a>**<font color="#f8805a">platform_timestamp</font>** (_float<small> - seconds</small>_)  
Time register of the frame at which this measurement was taken given by the OS in seconds.  

### Methods
- <a name="carla.Timestamp.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**frame**</font>, <font color="#00a6ed">**elapsed_seconds**</font>, <font color="#00a6ed">**delta_seconds**</font>, <font color="#00a6ed">**platform_timestamp**</font>)  
    - **Parameters:**
        - `frame` (_int_)  
        - `elapsed_seconds` (_float<small> - seconds</small>_)  
        - `delta_seconds` (_float<small> - seconds</small>_)  
        - `platform_timestamp` (_float<small> - seconds</small>_)  

##### Dunder methods
- <a name="carla.Timestamp.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Timestamp](#carla.Timestamp)</font>)  
- <a name="carla.Timestamp.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Timestamp](#carla.Timestamp)</font>)  
- <a name="carla.Timestamp.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.TrafficLight<a name="carla.TrafficLight"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.TrafficSign](#carla.TrafficSign)_</small></br>
A traffic light actor, considered a specific type of traffic sign. As traffic lights will mostly appear at junctions, they belong to a group which contains the different traffic lights in it. Inside the group, traffic lights are differenciated by their pole index.
     
  Within a group the state of traffic lights is changed in a cyclic pattern: one index is chosen and it spends a few seconds in green, yellow and eventually red. The rest of the traffic lights remain frozen in red this whole time, meaning that there is a gap in the last seconds of the cycle where all the traffic lights are red. However, the state of a traffic light can be changed manually.  

### Instance Variables
- <a name="carla.TrafficLight.state"></a>**<font color="#f8805a">state</font>** (_[carla.TrafficLightState](#carla.TrafficLightState)_)  
Current state of the traffic light.  

### Methods
- <a name="carla.TrafficLight.freeze"></a>**<font color="#7fb800">freeze</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**freeze**</font>)  
Stops all the traffic lights in the scene at their current state.  
    - **Parameters:**
        - `freeze` (_bool_)  
- <a name="carla.TrafficLight.is_frozen"></a>**<font color="#7fb800">is_frozen</font>**(<font color="#00a6ed">**self**</font>)  
The client returns <b>True</b> if a traffic light is frozen according to last tick. The method does not call the simulator.  
    - **Return:** _bool_  
- <a name="carla.TrafficLight.reset_group"></a>**<font color="#7fb800">reset_group</font>**(<font color="#00a6ed">**self**</font>)  
Resets the state of the traffic lights of the group to the initial state at the start of the simulation.  
    - **Note:** <font color="#8E8E8E">_This method calls the simulator.
_</font>  

##### Getters
- <a name="carla.TrafficLight.get_affected_lane_waypoints"></a>**<font color="#7fb800">get_affected_lane_waypoints</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of waypoints indicating the positions and lanes where the traffic light is having an effect.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.TrafficLight.get_elapsed_time"></a>**<font color="#7fb800">get_elapsed_time</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the time in seconds since current light state started according to last tick. The method does not call the simulator.  
    - **Return:** _float<small> - seconds</small>_  
- <a name="carla.TrafficLight.get_green_time"></a>**<font color="#7fb800">get_green_time</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the time set for the traffic light to be green, according to last tick. The method does not call the simulator.  
    - **Return:** _float<small> - seconds</small>_  
    - **Setter:** _[carla.TrafficLight.set_green_time](#carla.TrafficLight.set_green_time)_  
- <a name="carla.TrafficLight.get_group_traffic_lights"></a>**<font color="#7fb800">get_group_traffic_lights</font>**(<font color="#00a6ed">**self**</font>)  
Returns all traffic lights in the group this one belongs to.  
    - **Return:** _list([carla.TrafficLight](#carla.TrafficLight))_  
    - **Note:** <font color="#8E8E8E">_This method calls the simulator.
_</font>  
- <a name="carla.TrafficLight.get_light_boxes"></a>**<font color="#7fb800">get_light_boxes</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of the bounding boxes encapsulating each light box of the traffic light.  
    - **Return:** _list([carla.BoundingBox](#carla.BoundingBox))_  
- <a name="carla.TrafficLight.get_opendrive_id"></a>**<font color="#7fb800">get_opendrive_id</font>**(<font color="#00a6ed">**self**</font>)  
Returns the OpenDRIVE id of this traffic light.  
    - **Return:** _str_  
- <a name="carla.TrafficLight.get_pole_index"></a>**<font color="#7fb800">get_pole_index</font>**(<font color="#00a6ed">**self**</font>)  
Returns the index of the pole that identifies it as part of the traffic light group of a junction.  
    - **Return:** _int_  
- <a name="carla.TrafficLight.get_red_time"></a>**<font color="#7fb800">get_red_time</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the time set for the traffic light to be red, according to last tick. The method does not call the simulator.  
    - **Return:** _float<small> - seconds</small>_  
    - **Setter:** _[carla.TrafficLight.set_red_time](#carla.TrafficLight.set_red_time)_  
- <a name="carla.TrafficLight.get_state"></a>**<font color="#7fb800">get_state</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the state of the traffic light according to last tick. The method does not call the simulator.  
    - **Return:** _[carla.TrafficLightState](#carla.TrafficLightState)_  
    - **Setter:** _[carla.TrafficLight.set_state](#carla.TrafficLight.set_state)_  
- <a name="carla.TrafficLight.get_stop_waypoints"></a>**<font color="#7fb800">get_stop_waypoints</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of waypoints indicating the stop position for the traffic light. These waypoints are computed from the trigger boxes of the traffic light that indicate where a vehicle should stop.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.TrafficLight.get_yellow_time"></a>**<font color="#7fb800">get_yellow_time</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the time set for the traffic light to be yellow, according to last tick. The method does not call the simulator.  
    - **Return:** _float<small> - seconds</small>_  
    - **Setter:** _[carla.TrafficLight.set_yellow_time](#carla.TrafficLight.set_yellow_time)_  

##### Setters
- <a name="carla.TrafficLight.set_green_time"></a>**<font color="#7fb800">set_green_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**green_time**</font>)  
    - **Parameters:**
        - `green_time` (_float<small> - seconds</small>_) - Sets a given time for the green light to be active.  
    - **Getter:** _[carla.TrafficLight.get_green_time](#carla.TrafficLight.get_green_time)_  
- <a name="carla.TrafficLight.set_red_time"></a>**<font color="#7fb800">set_red_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**red_time**</font>)  
Sets a given time for the red state to be active.  
    - **Parameters:**
        - `red_time` (_float<small> - seconds</small>_)  
    - **Getter:** _[carla.TrafficLight.get_red_time](#carla.TrafficLight.get_red_time)_  
- <a name="carla.TrafficLight.set_state"></a>**<font color="#7fb800">set_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**state**</font>)<button class="SnipetButton" id="carla.TrafficLight.set_state-snipet_button">snippet &rarr;</button>  
Sets a given state to a traffic light actor.  
    - **Parameters:**
        - `state` (_[carla.TrafficLightState](#carla.TrafficLightState)_)  
    - **Getter:** _[carla.TrafficLight.get_state](#carla.TrafficLight.get_state)_  
- <a name="carla.TrafficLight.set_yellow_time"></a>**<font color="#7fb800">set_yellow_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**yellow_time**</font>)  
Sets a given time for the yellow light to be active.  
    - **Parameters:**
        - `yellow_time` (_float<small> - seconds</small>_)  
    - **Getter:** _[carla.TrafficLight.get_yellow_time](#carla.TrafficLight.get_yellow_time)_  

##### Dunder methods
- <a name="carla.TrafficLight.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.TrafficLightState<a name="carla.TrafficLightState"></a>
All possible states for traffic lights. These can either change at a specific time step or be changed manually. The snipet in [carla.TrafficLight.set_state](#carla.TrafficLight.set_state) changes the state of a traffic light on the fly.  

### Instance Variables
- <a name="carla.TrafficLightState.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.TrafficLightState.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.TrafficLightState.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.TrafficLightState.Off"></a>**<font color="#f8805a">Off</font>**  
- <a name="carla.TrafficLightState.Unknown"></a>**<font color="#f8805a">Unknown</font>**  

---

## carla.TrafficManager<a name="carla.TrafficManager"></a>
The traffic manager is a module built on top of the CARLA API in C++. It handles any group of vehicles set to autopilot mode to populate the simulation with realistic urban traffic conditions and give the chance to user to customize some behaviours. The architecture of the traffic manager is divided in five different goal-oriented stages and a PID controller where the information flows until eventually, a [carla.VehicleControl](#carla.VehicleControl) is applied to every vehicle registered in a traffic manager.
In order to learn more, visit the [documentation](adv_traffic_manager.md) regarding this module.  

### Methods
- <a name="carla.TrafficManager.auto_lane_change"></a>**<font color="#7fb800">auto_lane_change</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enable**</font>)  
Turns on or off lane changing behaviour for a vehicle.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The vehicle whose settings are changed.  
        - `enable` (_bool_) - __True__ is default and enables lane changes. __False__ will disable them.  
- <a name="carla.TrafficManager.collision_detection"></a>**<font color="#7fb800">collision_detection</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**reference_actor**</font>, <font color="#00a6ed">**other_actor**</font>, <font color="#00a6ed">**detect_collision**</font>)  
Tunes on/off collisions between a vehicle and another specific actor. In order to ignore all other vehicles, traffic lights or walkers, use the specific __ignore__ methods described in this same section.  
    - **Parameters:**
        - `reference_actor` (_[carla.Actor](#carla.Actor)_) - Vehicle that is going to ignore collisions.  
        - `other_actor` (_[carla.Actor](#carla.Actor)_) - The actor that `reference_actor` is going to ignore collisions with.  
        - `detect_collision` (_bool_) - __True__ is default and enables collisions. __False__ will disable them.  
- <a name="carla.TrafficManager.distance_to_leading_vehicle"></a>**<font color="#7fb800">distance_to_leading_vehicle</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**distance**</font>)  
Sets the minimum distance in meters that a vehicle has to keep with the others. The distance is in meters and will affect the minimum moving distance. It is computed from front to back of the vehicle objects.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle whose minimum distance is being changed.  
        - `distance` (_float<small> - meters</small>_) - Meters between both vehicles.  
- <a name="carla.TrafficManager.force_lane_change"></a>**<font color="#7fb800">force_lane_change</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**direction**</font>)  
Forces a vehicle to change either to the lane on its left or right, if existing, as indicated in `direction`. This method applies the lane change no matter what, disregarding possible collisions.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle being forced to change lanes.  
        - `direction` (_bool_) - Destination lane. __True__ is the one on the right and __False__ is the left one.  
- <a name="carla.TrafficManager.global_lane_offset"></a>**<font color="#7fb800">global_lane_offset</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**offset**</font>)  
Sets a global lane offset displacement from the center line. Positive values imply a right offset while negative ones mean a left one.
Default is 0. Numbers high enough to cause the vehicle to drive through other lanes might break the controller.  
    - **Parameters:**
        - `offset` (_float_) - Lane offset displacement from the center line.  
- <a name="carla.TrafficManager.global_percentage_speed_difference"></a>**<font color="#7fb800">global_percentage_speed_difference</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**percentage**</font>)  
Sets the difference the vehicle's intended speed and its current speed limit. Speed limits can be exceeded by setting the `perc` to a negative value.
Default is 30. Exceeding a speed limit can be done using negative percentages.  
    - **Parameters:**
        - `percentage` (_float_) - Percentage difference between intended speed and the current limit.  
- <a name="carla.TrafficManager.ignore_lights_percentage"></a>**<font color="#7fb800">ignore_lights_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**perc**</font>)  
During the traffic light stage, which runs every frame, this method sets the percent chance that traffic lights will be ignored for a vehicle.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that is going to ignore traffic lights.  
        - `perc` (_float_) - Between 0 and 100. Amount of times traffic lights will be ignored.  
- <a name="carla.TrafficManager.ignore_signs_percentage"></a>**<font color="#7fb800">ignore_signs_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**perc**</font>)  
During the traffic light stage, which runs every frame, this method sets the percent chance that stop signs will be ignored for a vehicle.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that is going to ignore stop signs.  
        - `perc` (_float_) - Between 0 and 100. Amount of times stop signs will be ignored.  
- <a name="carla.TrafficManager.ignore_vehicles_percentage"></a>**<font color="#7fb800">ignore_vehicles_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**perc**</font>)  
During the collision detection stage, which runs every frame, this method sets a percent chance that collisions with another vehicle will be ignored for a vehicle.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The vehicle that is going to ignore other vehicles.  
        - `perc` (_float_) - Between 0 and 100. Amount of times collisions will be ignored.  
- <a name="carla.TrafficManager.ignore_walkers_percentage"></a>**<font color="#7fb800">ignore_walkers_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**perc**</font>)  
During the collision detection stage, which runs every frame, this method sets a percent chance that collisions with walkers will be ignored for a vehicle.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The vehicle that is going to ignore walkers on scene.  
        - `perc` (_float_) - Between 0 and 100. Amount of times collisions will be ignored.  
- <a name="carla.TrafficManager.keep_right_rule_percentage"></a>**<font color="#7fb800">keep_right_rule_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**perc**</font>)  
During the localization stage, this method sets a percent chance that vehicle will follow the *keep right* rule, and stay in the right lane.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle whose behaviour is being changed.  
        - `perc` (_float_) - Between 0 and 100. Amount of times the vehicle will follow the keep right rule.  
- <a name="carla.TrafficManager.random_left_lanechange_percentage"></a>**<font color="#7fb800">random_left_lanechange_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**percentage**</font>)  
Adjust probability that in each timestep the actor will perform a left lane change, dependent on lane change availability.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that you wish to query.  
        - `percentage` (_float_) - The probability of lane change in percentage units (between 0 and 100).  
- <a name="carla.TrafficManager.random_right_lanechange_percentage"></a>**<font color="#7fb800">random_right_lanechange_percentage</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**percentage**</font>)  
Adjust probability that in each timestep the actor will perform a right lane change, dependent on lane change availability.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that you wish to query.  
        - `percentage` (_float_) - The probability of lane change in percentage units (between 0 and 100).  
- <a name="carla.TrafficManager.shut_down"></a>**<font color="#7fb800">shut_down</font>**(<font color="#00a6ed">**self**</font>)  
Shuts down the traffic manager.  
- <a name="carla.TrafficManager.update_vehicle_lights"></a>**<font color="#7fb800">update_vehicle_lights</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**do_update**</font>)  
Sets if the Traffic Manager is responsible of updating the vehicle lights, or not.
Default is __False__. The traffic manager will not change the vehicle light status of a vehicle, unless its auto_update_status is st to __True__.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle whose lights status is being changed.  
        - `do_update` (_bool_) - If __True__ the traffic manager will manage the vehicle lights for the specified vehicle.  
- <a name="carla.TrafficManager.vehicle_lane_offset"></a>**<font color="#7fb800">vehicle_lane_offset</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**offset**</font>)  
Sets a lane offset displacement from the center line. Positive values imply a right offset while negative ones mean a left one.
Default is 0. Numbers high enough to cause the vehicle to drive through other lanes might break the controller.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle whose lane offset behaviour is being changed.  
        - `offset` (_float_) - Lane offset displacement from the center line.  
- <a name="carla.TrafficManager.vehicle_percentage_speed_difference"></a>**<font color="#7fb800">vehicle_percentage_speed_difference</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**percentage**</font>)  
Sets the difference the vehicle's intended speed and its current speed limit. Speed limits can be exceeded by setting the `perc` to a negative value.
Default is 30. Exceeding a speed limit can be done using negative percentages.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle whose speed behaviour is being changed.  
        - `percentage` (_float_) - Percentage difference between intended speed and the current limit.  

##### Getters
- <a name="carla.TrafficManager.get_all_actions"></a>**<font color="#7fb800">get_all_actions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>)  
Returns all known actions (i.e. road options and waypoints) that an actor controlled by the Traffic Manager will perform in its next steps.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that you wish to query.  
    - **Return:** _list of lists with each element as follows - [Road option (string e.g. 'Left', 'Right', 'Straight'), Next waypoint ([carla.Waypoint](#carla.Waypoint))]_  
- <a name="carla.TrafficManager.get_next_action"></a>**<font color="#7fb800">get_next_action</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>)  
Returns the next known road option and waypoint that an actor controlled by the Traffic Manager will follow.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that you wish to query.  
    - **Return:** _list of two elements - [Road option (string e.g. 'Left', 'Right', 'Straight'), Next waypoint ([carla.Waypoint](#carla.Waypoint))]_  
- <a name="carla.TrafficManager.get_port"></a>**<font color="#7fb800">get_port</font>**(<font color="#00a6ed">**self**</font>)  
Returns the port where the Traffic Manager is connected. If the object is a TM-Client, it will return the port of its TM-Server. Read the [documentation](#adv_traffic_manager.md#multiclient-and-multitm-management) to learn the difference.  
    - **Return:** _uint16_  

##### Setters
- <a name="carla.TrafficManager.set_boundaries_respawn_dormant_vehicles"></a>**<font color="#7fb800">set_boundaries_respawn_dormant_vehicles</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lower_bound**=25.0</font>, <font color="#00a6ed">**upper_bound**=actor_active_distance</font>)  
Sets the upper and lower boundaries for dormant actors to be respawned near the hero vehicle.  
    - **Parameters:**
        - `lower_bound` (_float_) - The minimum distance in meters from the hero vehicle that a dormant actor will be respawned.  
        - `upper_bound` (_float_) - The maximum distance in meters from the hero vehicle that a dormant actor will be respawned.  
    - **Warning:** <font color="#ED2F2F">_The `upper_bound` cannot be higher than the `actor_active_distance`. The `lower_bound` cannot be less than 25.
_</font>  
- <a name="carla.TrafficManager.set_desired_speed"></a>**<font color="#7fb800">set_desired_speed</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**speed**</font>)  
Sets the speed of a vehicle to the specified value.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - Vehicle whose speed is being changed.  
        - `speed` (_float_) - Desired speed at which the vehicle will move.  
- <a name="carla.TrafficManager.set_global_distance_to_leading_vehicle"></a>**<font color="#7fb800">set_global_distance_to_leading_vehicle</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Sets the minimum distance in meters that vehicles have to keep with the rest. The distance is in meters and will affect the minimum moving distance. It is computed from center to center of the vehicle objects.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - Meters between vehicles.  
- <a name="carla.TrafficManager.set_hybrid_physics_mode"></a>**<font color="#7fb800">set_hybrid_physics_mode</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=False</font>)  
Enables or disables the hybrid physics mode. In this mode, vehicle's farther than a certain radius from the ego vehicle will have their physics disabled. Computation cost will be reduced by not calculating vehicle dynamics. Vehicles will be teleported.  
    - **Parameters:**
        - `enabled` (_bool_) - If __True__, enables the hybrid physics.  
- <a name="carla.TrafficManager.set_hybrid_physics_radius"></a>**<font color="#7fb800">set_hybrid_physics_radius</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**r**=50.0</font>)  
With hybrid physics on, changes the radius of the area of influence where physics are enabled.  
    - **Parameters:**
        - `r` (_float<small> - meters</small>_) - New radius where physics are enabled.  
- <a name="carla.TrafficManager.set_osm_mode"></a>**<font color="#7fb800">set_osm_mode</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**mode_switch**=True</font>)  
Enables or disables the OSM mode. This mode allows the user to run TM in a map created with the [OSM feature](tuto_G_openstreetmap.md). These maps allow having dead-end streets. Normally, if vehicles cannot find the next waypoint, TM crashes. If OSM mode is enabled, it will show a warning, and destroy vehicles when necessary.  
    - **Parameters:**
        - `mode_switch` (_bool_) - If __True__, the OSM mode is enabled.  
- <a name="carla.TrafficManager.set_path"></a>**<font color="#7fb800">set_path</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**path**</font>)  
Sets a list of locations for a vehicle to follow while controlled by the Traffic Manager.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that must follow the given path.  
        - `path` (_list_) - The list of [carla.Locations](#carla.Locations) for the actor to follow.  
    - **Warning:** <font color="#ED2F2F">_Ensure that the road topology doesn't impede the given path.
_</font>  
- <a name="carla.TrafficManager.set_random_device_seed"></a>**<font color="#7fb800">set_random_device_seed</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**value**</font>)  
Sets a specific random seed for the Traffic Manager, thereby setting it to be deterministic.  
    - **Parameters:**
        - `value` (_int_) - Seed value for the random number generation of the Traffic Manager.  
- <a name="carla.TrafficManager.set_respawn_dormant_vehicles"></a>**<font color="#7fb800">set_respawn_dormant_vehicles</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**mode_switch**=False</font>)  
If __True__, vehicles in large maps will respawn near the hero vehicle when they become dormant. Otherwise, they will stay dormant until they are within `actor_active_distance` of the hero vehicle again.  
    - **Parameters:**
        - `mode_switch` (_bool_)  
- <a name="carla.TrafficManager.set_route"></a>**<font color="#7fb800">set_route</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**path**</font>)  
Sets a list of route instructions for a vehicle to follow while controlled by the Traffic Manager. The possible route instructions are 'Left', 'Right', 'Straight'.  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor)_) - The actor that must follow the given route instructions.  
        - `path` (_list_) - The list of route instructions (string) for the vehicle to follow.  
    - **Warning:** <font color="#ED2F2F">_Ensure that the lane topology doesn't impede the given route.
_</font>  
- <a name="carla.TrafficManager.set_synchronous_mode"></a>**<font color="#7fb800">set_synchronous_mode</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**mode_switch**=True</font>)  
Sets the Traffic Manager to [synchronous mode](adv_traffic_manager.md#synchronous-mode). In a [multiclient situation](adv_traffic_manager.md#multiclient), only the TM-Server can tick. Similarly, in a [multiTM situation](adv_traffic_manager.md#multitm), only one TM-Server must tick. Use this method in the client that does the world tick, and right after setting the world to synchronous mode, to set which TM will be the master while in sync.  
    - **Parameters:**
        - `mode_switch` (_bool_) - If __True__, the TM synchronous mode is enabled.  
    - **Warning:** <font color="#ED2F2F">_If the server is set to synchronous mode, the TM <b>must</b> be set to synchronous mode too in the same client that does the tick.
_</font>  

---

## carla.TrafficSign<a name="carla.TrafficSign"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Actor](#carla.Actor)_</small></br>
Traffic signs appearing in the simulation except for traffic lights. These have their own class inherited from this in [carla.TrafficLight](#carla.TrafficLight). Right now, speed signs, stops and yields are mainly the ones implemented, but many others are borne in mind.  

### Instance Variables
- <a name="carla.TrafficSign.trigger_volume"></a>**<font color="#f8805a">trigger_volume</font>**  
A [carla.BoundingBox](#carla.BoundingBox) situated near a traffic sign where the [carla.Actor](#carla.Actor) who is inside can know about it.  

---

## carla.Transform<a name="carla.Transform"></a>
Class that defines a transformation, a combination of location and rotation, without scaling.  

### Instance Variables
- <a name="carla.Transform.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)_)  
Describes a point in the coordinate system.  
- <a name="carla.Transform.rotation"></a>**<font color="#f8805a">rotation</font>** (_[carla.Rotation](#carla.Rotation)<small> - degrees (pitch, yaw, roll)</small>_)  
Describes a rotation for an object according to Unreal Engine's axis system.  

### Methods
- <a name="carla.Transform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**rotation**</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `rotation` (_[carla.Rotation](#carla.Rotation)<small> - degrees (pitch, yaw, roll)</small>_)  
- <a name="carla.Transform.transform"></a>**<font color="#7fb800">transform</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**in_point**</font>)  
Translates a 3D point from local to global coordinates using the current transformation as frame of reference.  
    - **Parameters:**
        - `in_point` (_[carla.Location](#carla.Location)_) - Location in the space to which the transformation will be applied.  
- <a name="carla.Transform.transform_vector"></a>**<font color="#7fb800">transform_vector</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**in_vector**</font>)  
Rotates a vector using the current transformation as frame of reference, without applying translation. Use this to transform, for example, a velocity.  
    - **Parameters:**
        - `in_vector` (_[carla.Vector3D](#carla.Vector3D)_) - Vector to which the transformation will be applied.  

##### Getters
- <a name="carla.Transform.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes a forward vector using the rotation of the object.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Transform.get_inverse_matrix"></a>**<font color="#7fb800">get_inverse_matrix</font>**(<font color="#00a6ed">**self**</font>)  
Computes the 4-matrix representation of the inverse transformation.  
    - **Return:** _list(list(float))_  
- <a name="carla.Transform.get_matrix"></a>**<font color="#7fb800">get_matrix</font>**(<font color="#00a6ed">**self**</font>)  
Computes the 4-matrix representation of the transformation.  
    - **Return:** _list(list(float))_  
- <a name="carla.Transform.get_right_vector"></a>**<font color="#7fb800">get_right_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes a right vector using the rotation of the object.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Transform.get_up_vector"></a>**<font color="#7fb800">get_up_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes an up vector using the rotation of the object.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  

##### Dunder methods
- <a name="carla.Transform.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Transform](#carla.Transform)</font>)  
Returns __True__ if both location and rotation are equal for this and `other`.  
    - **Return:** _bool_  
- <a name="carla.Transform.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Transform](#carla.Transform)</font>)  
Returns __True__ if any location and rotation are not equal for this and `other`.  
    - **Return:** _bool_  
- <a name="carla.Transform.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses both location and rotation to string.  
    - **Return:** _str_  

---

## carla.Vector2D<a name="carla.Vector2D"></a>
Helper class to perform 2D operations.  

### Instance Variables
- <a name="carla.Vector2D.x"></a>**<font color="#f8805a">x</font>** (_float_)  
X-axis value.  
- <a name="carla.Vector2D.y"></a>**<font color="#f8805a">y</font>** (_float_)  
Y-axis value.  

### Methods
- <a name="carla.Vector2D.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
- <a name="carla.Vector2D.length"></a>**<font color="#7fb800">length</font>**(<font color="#00a6ed">**self**</font>)  
Computes the length of the vector.  
    - **Return:** _float_  
- <a name="carla.Vector2D.make_unit_vector"></a>**<font color="#7fb800">make_unit_vector</font>**(<font color="#00a6ed">**self**</font>)  
Returns a vector with the same direction and unitary length.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Vector2D.squared_length"></a>**<font color="#7fb800">squared_length</font>**(<font color="#00a6ed">**self**</font>)  
Computes the squared length of the vector.  
    - **Return:** _float_  

##### Dunder methods
- <a name="carla.Vector2D.__add__"></a>**<font color="#7fb800">\__add__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector2D](#carla.Vector2D)</font>)  
- <a name="carla.Vector2D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector2D](#carla.Vector2D)</font>)  
Returns __True__ if values for every axis are equal.  
    - **Return:** _bool_  
- <a name="carla.Vector2D.__mul__"></a>**<font color="#7fb800">\__mul__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector2D](#carla.Vector2D)</font>)  
- <a name="carla.Vector2D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**bool**=[carla.Vector2D](#carla.Vector2D)</font>)  
Returns __True__ if the value for any axis is different.  
    - **Return:** _bool_  
- <a name="carla.Vector2D.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Returns the axis values for the vector parsed as string.  
    - **Return:** _str_  
- <a name="carla.Vector2D.__sub__"></a>**<font color="#7fb800">\__sub__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector2D](#carla.Vector2D)</font>)  
- <a name="carla.Vector2D.__truediv__"></a>**<font color="#7fb800">\__truediv__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector2D](#carla.Vector2D)</font>)  

---

## carla.Vector3D<a name="carla.Vector3D"></a>
Helper class to perform 3D operations.  

### Instance Variables
- <a name="carla.Vector3D.x"></a>**<font color="#f8805a">x</font>** (_float_)  
X-axis value.  
- <a name="carla.Vector3D.y"></a>**<font color="#f8805a">y</font>** (_float_)  
Y-axis value.  
- <a name="carla.Vector3D.z"></a>**<font color="#f8805a">z</font>** (_float_)  
Z-axis value.  

### Methods
- <a name="carla.Vector3D.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>, <font color="#00a6ed">**z**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
        - `z` (_float_)  
- <a name="carla.Vector3D.cross"></a>**<font color="#7fb800">cross</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the cross product between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Vector3D.distance"></a>**<font color="#7fb800">distance</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the distance between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  
- <a name="carla.Vector3D.distance_2d"></a>**<font color="#7fb800">distance_2d</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the 2-dimensional distance between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  
- <a name="carla.Vector3D.distance_squared"></a>**<font color="#7fb800">distance_squared</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the squared distance between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  
- <a name="carla.Vector3D.distance_squared_2d"></a>**<font color="#7fb800">distance_squared_2d</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the 2-dimensional squared distance between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  
- <a name="carla.Vector3D.dot"></a>**<font color="#7fb800">dot</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the dot product between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  
- <a name="carla.Vector3D.dot_2d"></a>**<font color="#7fb800">dot_2d</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the 2-dimensional dot product between two vectors.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  
- <a name="carla.Vector3D.length"></a>**<font color="#7fb800">length</font>**(<font color="#00a6ed">**self**</font>)  
Computes the length of the vector.  
    - **Return:** _float_  
- <a name="carla.Vector3D.make_unit_vector"></a>**<font color="#7fb800">make_unit_vector</font>**(<font color="#00a6ed">**self**</font>)  
Returns a vector with the same direction and unitary length.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Vector3D.squared_length"></a>**<font color="#7fb800">squared_length</font>**(<font color="#00a6ed">**self**</font>)  
Computes the squared length of the vector.  
    - **Return:** _float_  

##### Getters
- <a name="carla.Vector3D.get_vector_angle"></a>**<font color="#7fb800">get_vector_angle</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**vector**</font>)  
Computes the angle between a pair of 3D vectors in radians.  
    - **Parameters:**
        - `vector` (_[carla.Vector3D](#carla.Vector3D)_)  
    - **Return:** _float_  

##### Dunder methods
- <a name="carla.Vector3D.__abs__"></a>**<font color="#7fb800">\__abs__</font>**(<font color="#00a6ed">**self**</font>)  
Returns a Vector3D with the absolute value of the components x, y and z.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Vector3D.__add__"></a>**<font color="#7fb800">\__add__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector3D](#carla.Vector3D)</font>)  
- <a name="carla.Vector3D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector3D](#carla.Vector3D)</font>)  
Returns __True__ if values for every axis are equal.  
    - **Return:** _bool_  
- <a name="carla.Vector3D.__mul__"></a>**<font color="#7fb800">\__mul__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector3D](#carla.Vector3D)</font>)  
- <a name="carla.Vector3D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector3D](#carla.Vector3D)</font>)  
Returns __True__ if the value for any axis is different.  
    - **Return:** _bool_  
- <a name="carla.Vector3D.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Returns the axis values for the vector parsed as string.  
    - **Return:** _str_  
- <a name="carla.Vector3D.__sub__"></a>**<font color="#7fb800">\__sub__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector3D](#carla.Vector3D)</font>)  
- <a name="carla.Vector3D.__truediv__"></a>**<font color="#7fb800">\__truediv__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.Vector3D](#carla.Vector3D)</font>)  

---

## carla.Vehicle<a name="carla.Vehicle"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Actor](#carla.Actor)_</small></br>
One of the most important groups of actors in CARLA. These include any type of vehicle from cars to trucks, motorbikes, vans, bycicles and also official vehicles such as police cars. A wide set of these actors is provided in [carla.BlueprintLibrary](#carla.BlueprintLibrary) to facilitate differente requirements. Vehicles can be either manually controlled or set to an autopilot mode that will be conducted client-side by the <b>traffic manager</b>.  

### Instance Variables
- <a name="carla.Vehicle.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
Bounding box containing the geometry of the vehicle. Its location and rotation are relative to the vehicle it is attached to.  

### Methods
- <a name="carla.Vehicle.apply_ackermann_control"></a>**<font color="#7fb800">apply_ackermann_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Applies an Ackermann control object on the next tick.  
    - **Parameters:**
        - `control` (_[carla.VehicleAckermannControl](#carla.VehicleAckermannControl)_)  
- <a name="carla.Vehicle.apply_ackermann_controller_settings"></a>**<font color="#7fb800">apply_ackermann_controller_settings</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**settings**</font>)  
Applies a new Ackermann control settings to this vehicle in the next tick.  
    - **Parameters:**
        - `settings` (_[carla.AckermannControllerSettings](#carla.AckermannControllerSettings)_)  
    - **Warning:** <font color="#ED2F2F">_This method does call the simulator._</font>  
- <a name="carla.Vehicle.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Applies a control object on the next tick, containing driving parameters such as throttle, steering or gear shifting.  
    - **Parameters:**
        - `control` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.Vehicle.apply_physics_control"></a>**<font color="#7fb800">apply_physics_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**physics_control**</font>)  
Applies a physics control object in the next tick containing the parameters that define the vehicle as a corporeal body. E.g.: moment of inertia, mass, drag coefficient and many more.  
    - **Parameters:**
        - `physics_control` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.Vehicle.close_door"></a>**<font color="#7fb800">close_door</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**door_idx**</font>)  
Close the door `door_idx` if the vehicle has it. Use [carla.VehicleDoor.All](#carla.VehicleDoor.All) to close all available doors.  
    - **Parameters:**
        - `door_idx` (_[carla.VehicleDoor](#carla.VehicleDoor)_) - door index.  
- <a name="carla.Vehicle.enable_carsim"></a>**<font color="#7fb800">enable_carsim</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**simfile_path**</font>)  
Enables the CarSim physics solver for this particular vehicle. In order for this function to work, there needs to be a valid license manager running on the server side. The control inputs are redirected to CarSim which will provide the position and orientation of the vehicle for every frame.  
    - **Parameters:**
        - `simfile_path` (_str_) - Path to the `.simfile` file with the parameters of the simulation.  
- <a name="carla.Vehicle.enable_chrono_physics"></a>**<font color="#7fb800">enable_chrono_physics</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**max_substeps**</font>, <font color="#00a6ed">**max_substep_delta_time**</font>, <font color="#00a6ed">**vehicle_json**</font>, <font color="#00a6ed">**powertrain_json**</font>, <font color="#00a6ed">**tire_json**</font>, <font color="#00a6ed">**base_json_path**</font>)  
Enables Chrono physics on a spawned vehicle.  
    - **Parameters:**
        - `max_substeps` (_int_) - Max number of Chrono substeps.  
        - `max_substep_delta_time` (_int_) - Max size of substep.  
        - `vehicle_json` (_str_) - Path to vehicle json file relative to `base_json_path`.  
        - `powertrain_json` (_str_) - Path to powertrain json file relative to `base_json_path`.  
        - `tire_json` (_str_) - Path to tire json file relative to `base_json_path`.  
        - `base_json_path` (_str_) - Path to `chrono/data/vehicle` folder. E.g., `/home/user/carla/Build/chrono-install/share/chrono/data/vehicle/` (the final `/` character is required).  
    - **Note:** <font color="#8E8E8E">_Ensure that you have started the CARLA server with the `ARGS="--chrono"` flag. You will not be able to use Chrono physics without this flag set.
_</font>  
    - **Warning:** <font color="#ED2F2F">_Collisions are not supported. When a collision is detected, physics will revert to the default CARLA physics.
_</font>  
- <a name="carla.Vehicle.is_at_traffic_light"></a>**<font color="#7fb800">is_at_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
Vehicles will be affected by a traffic light when the light is red and the vehicle is inside its bounding box. The client returns whether a traffic light is affecting this vehicle according to last tick (it does not call the simulator).  
    - **Return:** _bool_  
- <a name="carla.Vehicle.open_door"></a>**<font color="#7fb800">open_door</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**door_idx**</font>)  
Open the door `door_idx` if the vehicle has it. Use [carla.VehicleDoor.All](#carla.VehicleDoor.All) to open all available doors.  
    - **Parameters:**
        - `door_idx` (_[carla.VehicleDoor](#carla.VehicleDoor)_) - door index.  
- <a name="carla.Vehicle.show_debug_telemetry"></a>**<font color="#7fb800">show_debug_telemetry</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>)  
Enables or disables the telemetry on this vehicle. This shows information about the vehicles current state and forces applied to it in the spectator window. Only information for one vehicle can be shown so that, if you enable a second one, the previous will be automatically disabled.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Vehicle.use_carsim_road"></a>**<font color="#7fb800">use_carsim_road</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**</font>)  
Enables or disables the usage of CarSim vs terrain file specified in the `.simfile`. By default this option is disabled and CarSim uses unreal engine methods to process the geometry of the scene.  
    - **Parameters:**
        - `enabled` (_bool_)  

##### Getters
- <a name="carla.Vehicle.get_ackermann_controller_settings"></a>**<font color="#7fb800">get_ackermann_controller_settings</font>**(<font color="#00a6ed">**self**</font>)  
Returns the last Ackermann control settings applied to this vehicle.  
    - **Return:** _[carla.AckermannControllerSettings](#carla.AckermannControllerSettings)_  
    - **Warning:** <font color="#ED2F2F">_This method does call the simulator to retrieve the value._</font>  
- <a name="carla.Vehicle.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the control applied in the last tick. The method does not call the simulator.  
    - **Return:** _[carla.VehicleControl](#carla.VehicleControl)_  
- <a name="carla.Vehicle.get_failure_state"></a>**<font color="#7fb800">get_failure_state</font>**(<font color="#00a6ed">**self**</font>)  
Vehicle have failure states, to  indicate that it is incapable of continuing its route. This function returns the vehicle's specific failure state, or in other words, the cause that resulted in it.  
    - **Return:** _[carla.VehicleFailureState](#carla.VehicleFailureState)_  
- <a name="carla.Vehicle.get_light_state"></a>**<font color="#7fb800">get_light_state</font>**(<font color="#00a6ed">**self**</font>)  
Returns a flag representing the vehicle light state, this represents which lights are active or not.  
    - **Return:** _[carla.VehicleLightState](#carla.VehicleLightState)_  
    - **Setter:** _[carla.Vehicle.set_light_state](#carla.Vehicle.set_light_state)_  
- <a name="carla.Vehicle.get_physics_control"></a>**<font color="#7fb800">get_physics_control</font>**(<font color="#00a6ed">**self**</font>)  
The simulator returns the last physics control applied to this vehicle.  
    - **Return:** _[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_  
    - **Warning:** <font color="#ED2F2F">_This method does call the simulator to retrieve the value._</font>  
- <a name="carla.Vehicle.get_speed_limit"></a>**<font color="#7fb800">get_speed_limit</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the speed limit affecting this vehicle according to last tick (it does not call the simulator). The speed limit is updated when passing by a speed limit signal, so a vehicle might have none right after spawning.  
    - **Return:** _float<small> - km/h</small>_  
- <a name="carla.Vehicle.get_traffic_light"></a>**<font color="#7fb800">get_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
Retrieves the traffic light actor affecting this vehicle (if any) according to last tick. The method does not call the simulator.  
    - **Return:** _[carla.TrafficLight](#carla.TrafficLight)_  
- <a name="carla.Vehicle.get_traffic_light_state"></a>**<font color="#7fb800">get_traffic_light_state</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the state of the traffic light affecting this vehicle according to last tick. The method does not call the simulator. If no traffic light is currently affecting the vehicle, returns <b>green</b>.  
    - **Return:** _[carla.TrafficLightState](#carla.TrafficLightState)_  
- <a name="carla.Vehicle.get_wheel_steer_angle"></a>**<font color="#7fb800">get_wheel_steer_angle</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wheel_location**</font>)  
Returns the physics angle in degrees of a vehicle's wheel.  
    - **Parameters:**
        - `wheel_location` (_[carla.VehicleWheelLocation](#carla.VehicleWheelLocation)_)  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_Returns the angle based on the physics of the wheel, not the visual angle.
_</font>  

##### Setters
- <a name="carla.Vehicle.set_autopilot"></a>**<font color="#7fb800">set_autopilot</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>, <font color="#00a6ed">**port**=8000</font>)  
Registers or deletes the vehicle from a Traffic Manager's list. When __True__, the Traffic Manager passed as parameter will move the vehicle around. The autopilot takes place client-side.  
    - **Parameters:**
        - `enabled` (_bool_)  
        - `port` (_uint16_) - The port of the TM-Server where the vehicle is to be registered or unlisted. If __None__ is passed, it will consider a TM at default port `8000`.  
- <a name="carla.Vehicle.set_light_state"></a>**<font color="#7fb800">set_light_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**light_state**</font>)  
Sets the light state of a vehicle using a flag that represents the lights that are on and off.  
    - **Parameters:**
        - `light_state` (_[carla.VehicleLightState](#carla.VehicleLightState)_)  
    - **Getter:** _[carla.Vehicle.get_light_state](#carla.Vehicle.get_light_state)_  
- <a name="carla.Vehicle.set_wheel_steer_direction"></a>**<font color="#7fb800">set_wheel_steer_direction</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wheel_location**</font>, <font color="#00a6ed">**angle_in_deg**</font>)<button class="SnipetButton" id="carla.Vehicle.set_wheel_steer_direction-snipet_button">snippet &rarr;</button>  
Sets the angle of a vehicle's wheel visually.  
    - **Parameters:**
        - `wheel_location` (_[carla.VehicleWheelLocation](#carla.VehicleWheelLocation)_)  
        - `angle_in_deg` (_float_)  
    - **Warning:** <font color="#ED2F2F">_Does not affect the physics of the vehicle.
_</font>  

##### Dunder methods
- <a name="carla.Vehicle.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehicleAckermannControl<a name="carla.VehicleAckermannControl"></a>
Manages the basic movement of a vehicle using Ackermann driving controls.  

### Instance Variables
- <a name="carla.VehicleAckermannControl.steer"></a>**<font color="#f8805a">steer</font>** (_float_)  
Desired steer (rad). Positive value is to the right. Default is 0.0.  
- <a name="carla.VehicleAckermannControl.steer_speed"></a>**<font color="#f8805a">steer_speed</font>** (_float_)  
Steering velocity (rad/s). Zero steering angle velocity means change the steering angle as quickly as possible. Default is 0.0.  
- <a name="carla.VehicleAckermannControl.speed"></a>**<font color="#f8805a">speed</font>** (_float_)  
Desired speed (m/s). Default is 0.0.  
- <a name="carla.VehicleAckermannControl.acceleration"></a>**<font color="#f8805a">acceleration</font>** (_float_)  
Desired acceleration (m/s2) Default is 0.0.  
- <a name="carla.VehicleAckermannControl.jerk"></a>**<font color="#f8805a">jerk</font>** (_float_)  
Desired jerk (m/s3). Default is 0.0.  

### Methods
- <a name="carla.VehicleAckermannControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**steer**=0.0</font>, <font color="#00a6ed">**steer_speed**=0.0</font>, <font color="#00a6ed">**speed**=0.0</font>, <font color="#00a6ed">**acceleration**=0.0</font>, <font color="#00a6ed">**jerk**=0.0</font>)  
    - **Parameters:**
        - `steer` (_float_)  
        - `steer_speed` (_float_)  
        - `speed` (_float_)  
        - `acceleration` (_float_)  
        - `jerk` (_float_)  

##### Dunder methods
- <a name="carla.VehicleAckermannControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.AckermannVehicleControl](#carla.AckermannVehicleControl)</font>)  
- <a name="carla.VehicleAckermannControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.AckermannVehicleControl](#carla.AckermannVehicleControl)</font>)  
- <a name="carla.VehicleAckermannControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehicleControl<a name="carla.VehicleControl"></a>
Manages the basic movement of a vehicle using typical driving controls.  

### Instance Variables
- <a name="carla.VehicleControl.throttle"></a>**<font color="#f8805a">throttle</font>** (_float_)  
A scalar value to control the vehicle throttle [0.0, 1.0]. Default is 0.0.  
- <a name="carla.VehicleControl.steer"></a>**<font color="#f8805a">steer</font>** (_float_)  
A scalar value to control the vehicle steering [-1.0, 1.0]. Default is 0.0.  
- <a name="carla.VehicleControl.brake"></a>**<font color="#f8805a">brake</font>** (_float_)  
A scalar value to control the vehicle brake [0.0, 1.0]. Default is 0.0.  
- <a name="carla.VehicleControl.hand_brake"></a>**<font color="#f8805a">hand_brake</font>** (_bool_)  
Determines whether hand brake will be used. Default is <b>False</b>.  
- <a name="carla.VehicleControl.reverse"></a>**<font color="#f8805a">reverse</font>** (_bool_)  
Determines whether the vehicle will move backwards. Default is <b>False</b>.  
- <a name="carla.VehicleControl.manual_gear_shift"></a>**<font color="#f8805a">manual_gear_shift</font>** (_bool_)  
Determines whether the vehicle will be controlled by changing gears manually. Default is <b>False</b>.  
- <a name="carla.VehicleControl.gear"></a>**<font color="#f8805a">gear</font>** (_int_)  
States which gear is the vehicle running on.  

### Methods
- <a name="carla.VehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**throttle**=0.0</font>, <font color="#00a6ed">**steer**=0.0</font>, <font color="#00a6ed">**brake**=0.0</font>, <font color="#00a6ed">**hand_brake**=False</font>, <font color="#00a6ed">**reverse**=False</font>, <font color="#00a6ed">**manual_gear_shift**=False</font>, <font color="#00a6ed">**gear**=0</font>)  
    - **Parameters:**
        - `throttle` (_float_) - Scalar value between [0.0,1.0].  
        - `steer` (_float_) - Scalar value between [0.0,1.0].  
        - `brake` (_float_) - Scalar value between [0.0,1.0].  
        - `hand_brake` (_bool_)  
        - `reverse` (_bool_)  
        - `manual_gear_shift` (_bool_)  
        - `gear` (_int_)  

##### Dunder methods
- <a name="carla.VehicleControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.VehicleControl](#carla.VehicleControl)</font>)  
- <a name="carla.VehicleControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.VehicleControl](#carla.VehicleControl)</font>)  
- <a name="carla.VehicleControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehicleDoor<a name="carla.VehicleDoor"></a>
Possible index representing the possible doors that can be open. Notice that not all possible doors are able to open in some vehicles.  

### Instance Variables
- <a name="carla.VehicleDoor.FL"></a>**<font color="#f8805a">FL</font>**  
Front left door.  
- <a name="carla.VehicleDoor.FR"></a>**<font color="#f8805a">FR</font>**  
Front right door.  
- <a name="carla.VehicleDoor.RL"></a>**<font color="#f8805a">RL</font>**  
Back left door.  
- <a name="carla.VehicleDoor.RR"></a>**<font color="#f8805a">RR</font>**  
Back right door.  
- <a name="carla.VehicleDoor.All"></a>**<font color="#f8805a">All</font>**  
Represents all doors.  

---

## carla.VehicleFailureState<a name="carla.VehicleFailureState"></a>
Enum containing the different failure states of a vehicle, from which the it cannot recover. These are returned by __<font color="#7fb800">get_failure_state()</font>__ and only Rollover is currently implemented.  

### Instance Variables
- <a name="carla.VehicleFailureState.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.VehicleFailureState.Rollover"></a>**<font color="#f8805a">Rollover</font>**  
- <a name="carla.VehicleFailureState.Engine"></a>**<font color="#f8805a">Engine</font>**  
- <a name="carla.VehicleFailureState.TirePuncture"></a>**<font color="#f8805a">TirePuncture</font>**  

---

## carla.VehicleLightState<a name="carla.VehicleLightState"></a>
Class that recaps the state of the lights of a vehicle, these can be used as a flags. E.g: `VehicleLightState.HighBeam & VehicleLightState.Brake` will return `True` when both are active. Lights are off by default in any situation and should be managed by the user via script. The blinkers blink automatically. _Warning: Right now, not all vehicles have been prepared to work with this functionality, this will be added to all of them in later updates_.  

### Instance Variables
- <a name="carla.VehicleLightState.NONE"></a>**<font color="#f8805a">NONE</font>**  
All lights off.  
- <a name="carla.VehicleLightState.Position"></a>**<font color="#f8805a">Position</font>**  
- <a name="carla.VehicleLightState.LowBeam"></a>**<font color="#f8805a">LowBeam</font>**  
- <a name="carla.VehicleLightState.HighBeam"></a>**<font color="#f8805a">HighBeam</font>**  
- <a name="carla.VehicleLightState.Brake"></a>**<font color="#f8805a">Brake</font>**  
- <a name="carla.VehicleLightState.RightBlinker"></a>**<font color="#f8805a">RightBlinker</font>**  
- <a name="carla.VehicleLightState.LeftBlinker"></a>**<font color="#f8805a">LeftBlinker</font>**  
- <a name="carla.VehicleLightState.Reverse"></a>**<font color="#f8805a">Reverse</font>**  
- <a name="carla.VehicleLightState.Fog"></a>**<font color="#f8805a">Fog</font>**  
- <a name="carla.VehicleLightState.Interior"></a>**<font color="#f8805a">Interior</font>**  
- <a name="carla.VehicleLightState.Special1"></a>**<font color="#f8805a">Special1</font>**  
This is reserved for certain vehicles that can have special lights, like a siren.  
- <a name="carla.VehicleLightState.Special2"></a>**<font color="#f8805a">Special2</font>**  
This is reserved for certain vehicles that can have special lights, like a siren.  
- <a name="carla.VehicleLightState.All"></a>**<font color="#f8805a">All</font>**  
All lights on.  

---

## carla.VehiclePhysicsControl<a name="carla.VehiclePhysicsControl"></a>
Summarizes the parameters that will be used to simulate a [carla.Vehicle](#carla.Vehicle) as a physical object. The specific settings for the wheels though are stipulated using [carla.WheelPhysicsControl](#carla.WheelPhysicsControl).  

### Instance Variables
- <a name="carla.VehiclePhysicsControl.torque_curve"></a>**<font color="#f8805a">torque_curve</font>** (_list([carla.Vector2D](#carla.Vector2D))_)  
Curve that indicates the torque measured in Nm for a specific RPM of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.max_rpm"></a>**<font color="#f8805a">max_rpm</font>** (_float_)  
The maximum RPM of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.moi"></a>**<font color="#f8805a">moi</font>** (_float<small> - kg*m<sup>2</sup></small>_)  
The moment of inertia of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.damping_rate_full_throttle"></a>**<font color="#f8805a">damping_rate_full_throttle</font>** (_float_)  
Damping ratio when the throttle is maximum.  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_engaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_engaged</font>** (_float_)  
Damping ratio when the throttle is zero with clutch engaged.  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_disengaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_disengaged</font>** (_float_)  
Damping ratio when the throttle is zero with clutch disengaged.  
- <a name="carla.VehiclePhysicsControl.use_gear_autobox"></a>**<font color="#f8805a">use_gear_autobox</font>** (_bool_)  
If <b>True</b>, the vehicle will have an automatic transmission.  
- <a name="carla.VehiclePhysicsControl.gear_switch_time"></a>**<font color="#f8805a">gear_switch_time</font>** (_float<small> - seconds</small>_)  
Switching time between gears.  
- <a name="carla.VehiclePhysicsControl.clutch_strength"></a>**<font color="#f8805a">clutch_strength</font>** (_float<small> - kg*m<sup>2</sup>/s</small>_)  
Clutch strength of the vehicle.  
- <a name="carla.VehiclePhysicsControl.final_ratio"></a>**<font color="#f8805a">final_ratio</font>** (_float_)  
Fixed ratio from transmission to wheels.  
- <a name="carla.VehiclePhysicsControl.forward_gears"></a>**<font color="#f8805a">forward_gears</font>** (_list([carla.GearPhysicsControl](#carla.GearPhysicsControl))_)  
List of objects defining the vehicle's gears.  
- <a name="carla.VehiclePhysicsControl.mass"></a>**<font color="#f8805a">mass</font>** (_float<small> - kilograms</small>_)  
Mass of the vehicle.  
- <a name="carla.VehiclePhysicsControl.drag_coefficient"></a>**<font color="#f8805a">drag_coefficient</font>** (_float_)  
Drag coefficient of the vehicle's chassis.  
- <a name="carla.VehiclePhysicsControl.center_of_mass"></a>**<font color="#f8805a">center_of_mass</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - meters</small>_)  
Center of mass of the vehicle.  
- <a name="carla.VehiclePhysicsControl.steering_curve"></a>**<font color="#f8805a">steering_curve</font>** (_list([carla.Vector2D](#carla.Vector2D))_)  
Curve that indicates the maximum steering for a specific forward speed.  
- <a name="carla.VehiclePhysicsControl.use_sweep_wheel_collision"></a>**<font color="#f8805a">use_sweep_wheel_collision</font>** (_bool_)  
Enable the use of sweep for wheel collision. By default, it is disabled and it uses a simple raycast from the axis to the floor for each wheel. This option provides a better collision model in which the full volume of the wheel is checked against collisions.  
- <a name="carla.VehiclePhysicsControl.wheels"></a>**<font color="#f8805a">wheels</font>** (_list([carla.WheelPhysicsControl](#carla.WheelPhysicsControl))_)  
List of wheel physics objects. This list should have 4 elements, where index 0 corresponds to the front left wheel, index 1 corresponds to the front right wheel, index 2 corresponds to the back left wheel and index 3 corresponds to the back right wheel. For 2 wheeled vehicles, set the same values for both front and back wheels.  

### Methods
- <a name="carla.VehiclePhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**torque_curve**=[[0.0, 500.0], [5000.0, 500.0]]</font>, <font color="#00a6ed">**max_rpm**=5000.0</font>, <font color="#00a6ed">**moi**=1.0</font>, <font color="#00a6ed">**damping_rate_full_throttle**=0.15</font>, <font color="#00a6ed">**damping_rate_zero_throttle_clutch_engaged**=2.0</font>, <font color="#00a6ed">**damping_rate_zero_throttle_clutch_disengaged**=0.35</font>, <font color="#00a6ed">**use_gear_autobox**=True</font>, <font color="#00a6ed">**gear_switch_time**=0.5</font>, <font color="#00a6ed">**clutch_strength**=10.0</font>, <font color="#00a6ed">**final_ratio**=4.0</font>, <font color="#00a6ed">**forward_gears**=list()</font>, <font color="#00a6ed">**drag_coefficient**=0.3</font>, <font color="#00a6ed">**center_of_mass**=[0.0, 0.0, 0.0]</font>, <font color="#00a6ed">**steering_curve**=[[0.0, 1.0], [10.0, 0.5]]</font>, <font color="#00a6ed">**wheels**=list()</font>, <font color="#00a6ed">**use_sweep_wheel_collision**=False</font>, <font color="#00a6ed">**mass**=1000.0</font>)  
VehiclePhysicsControl constructor.  
    - **Parameters:**
        - `torque_curve` (_list([carla.Vector2D](#carla.Vector2D))_)  
        - `max_rpm` (_float_)  
        - `moi` (_float<small> - kg*m<sup>2</sup></small>_)  
        - `damping_rate_full_throttle` (_float_)  
        - `damping_rate_zero_throttle_clutch_engaged` (_float_)  
        - `damping_rate_zero_throttle_clutch_disengaged` (_float_)  
        - `use_gear_autobox` (_bool_)  
        - `gear_switch_time` (_float<small> - seconds</small>_)  
        - `clutch_strength` (_float<small> - kg*m<sup>2</sup>/s</small>_)  
        - `final_ratio` (_float_)  
        - `forward_gears` (_list([carla.GearPhysicsControl](#carla.GearPhysicsControl))_)  
        - `drag_coefficient` (_float_)  
        - `center_of_mass` (_[carla.Vector3D](#carla.Vector3D)_)  
        - `steering_curve` (_[carla.Vector2D](#carla.Vector2D)_)  
        - `wheels` (_list([carla.WheelPhysicsControl](#carla.WheelPhysicsControl))_)  
        - `use_sweep_wheel_collision` (_bool_)  
        - `mass` (_float<small> - kilograms</small>_)  

##### Dunder methods
- <a name="carla.VehiclePhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)</font>)  
- <a name="carla.VehiclePhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)</font>)  
- <a name="carla.VehiclePhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehicleWheelLocation<a name="carla.VehicleWheelLocation"></a>
`enum` representing the position of each wheel on a vehicle.  Used to identify the target wheel when setting an angle in [carla.Vehicle.set_wheel_steer_direction](#carla.Vehicle.set_wheel_steer_direction) or [carla.Vehicle.get_wheel_steer_angle](#carla.Vehicle.get_wheel_steer_angle).  

### Instance Variables
- <a name="carla.VehicleWheelLocation.FL_Wheel"></a>**<font color="#f8805a">FL_Wheel</font>**  
Front left wheel of a 4 wheeled vehicle.  
- <a name="carla.VehicleWheelLocation.FR_Wheel"></a>**<font color="#f8805a">FR_Wheel</font>**  
Front right wheel of a 4 wheeled vehicle.  
- <a name="carla.VehicleWheelLocation.BL_Wheel"></a>**<font color="#f8805a">BL_Wheel</font>**  
Back left wheel of a 4 wheeled vehicle.  
- <a name="carla.VehicleWheelLocation.BR_Wheel"></a>**<font color="#f8805a">BR_Wheel</font>**  
Back right wheel of a 4 wheeled vehicle.  
- <a name="carla.VehicleWheelLocation.Front_Wheel"></a>**<font color="#f8805a">Front_Wheel</font>**  
Front wheel of a 2 wheeled vehicle.  
- <a name="carla.VehicleWheelLocation.Back_Wheel"></a>**<font color="#f8805a">Back_Wheel</font>**  
Back wheel of a 2 wheeled vehicle.  

---

## carla.Walker<a name="carla.Walker"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Actor](#carla.Actor)_</small></br>
This class inherits from the [carla.Actor](#carla.Actor) and defines pedestrians in the simulation. Walkers are a special type of actor that can be controlled either by an AI ([carla.WalkerAIController](#carla.WalkerAIController)) or manually via script, using a series of [carla.WalkerControl](#carla.WalkerControl) to move these and their skeletons.  

### Methods
- <a name="carla.Walker.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
On the next tick, the control will move the walker in a certain direction with a certain speed. Jumps can be commanded too.  
    - **Parameters:**
        - `control` (_[carla.WalkerControl](#carla.WalkerControl)_)  
- <a name="carla.Walker.blend_pose"></a>**<font color="#7fb800">blend_pose</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blend_value**</font>)  
Set the blending value of the custom pose with the animation. The values can be:
  - 0: will show only the animation
  - 1: will show only the custom pose (set by the user with set_bones())
  - any other: will interpolate all the bone positions between animation and the custom pose.  
    - **Parameters:**
        - `blend_value` (_float<small> - value from 0 to 1 with the blend percentage</small>_)  
- <a name="carla.Walker.hide_pose"></a>**<font color="#7fb800">hide_pose</font>**(<font color="#00a6ed">**self**</font>)  
Hide the custom pose and show the animation (same as calling blend_pose(0)).  
- <a name="carla.Walker.show_pose"></a>**<font color="#7fb800">show_pose</font>**(<font color="#00a6ed">**self**</font>)  
Show the custom pose and hide the animation (same as calling blend_pose(1)).  

##### Getters
- <a name="carla.Walker.get_bones"></a>**<font color="#7fb800">get_bones</font>**(<font color="#00a6ed">**self**</font>)  
Return the structure with all the bone transformations from the actor. For each bone, we get the name and its transform in three different spaces:
  - name: bone name
  - world: transform in world coordinates
  - component: transform based on the pivot of the actor
  - relative: transform based on the bone parent.  
    - **Return:** _[carla.WalkerBoneControlOut](#carla.WalkerBoneControlOut)_  
    - **Setter:** _[carla.Walker.set_bones](#carla.Walker.set_bones)_  
- <a name="carla.Walker.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
The client returns the control applied to this walker during last tick. The method does not call the simulator.  
    - **Return:** _[carla.WalkerControl](#carla.WalkerControl)_  
- <a name="carla.Walker.get_pose_from_animation"></a>**<font color="#7fb800">get_pose_from_animation</font>**(<font color="#00a6ed">**self**</font>)  
Make a copy of the current animation frame as the custom pose. Initially the custom pose is the neutral pedestrian pose.  

##### Setters
- <a name="carla.Walker.set_bones"></a>**<font color="#7fb800">set_bones</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**bones**</font>)  
Set the bones of the actor. For each bone we want to set we use a relative transform. Only the bones in this list will be set. For each bone you need to setup this info:
  - name: bone name
  - relative: transform based on the bone parent.  
    - **Parameters:**
        - `bones` (_[carla.WalkerBoneControlIn](#carla.WalkerBoneControlIn)<small> - list of pairs (bone_name, transform) for the bones that we want to set</small>_)  
    - **Getter:** _[carla.Walker.get_bones](#carla.Walker.get_bones)_  

##### Dunder methods
- <a name="carla.Walker.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerAIController<a name="carla.WalkerAIController"></a>
<small style="display:block;margin-top:-20px;">Inherited from _[carla.Actor](#carla.Actor)_</small></br>
Class that conducts AI control for a walker. The controllers are defined as actors, but they are quite different from the rest. They need to be attached to a parent actor during their creation, which is the walker they will be controlling (take a look at [carla.World](#carla.World) if you are yet to learn on how to spawn actors). They also need for a special blueprint (already defined in [carla.BlueprintLibrary](#carla.BlueprintLibrary) as "controller.ai.walker"). This is an empty blueprint, as the AI controller will be invisible in the simulation but will follow its parent around to dictate every step of the way.  

### Methods
- <a name="carla.WalkerAIController.go_to_location"></a>**<font color="#7fb800">go_to_location</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**destination**</font>)  
Sets the destination that the pedestrian will reach.  
    - **Parameters:**
        - `destination` (_[carla.Location](#carla.Location)<small> - meters</small>_)  
- <a name="carla.WalkerAIController.start"></a>**<font color="#7fb800">start</font>**(<font color="#00a6ed">**self**</font>)  
Enables AI control for its parent walker.  
- <a name="carla.WalkerAIController.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)<button class="SnipetButton" id="carla.WalkerAIController.stop-snipet_button">snippet &rarr;</button>  
Disables AI control for its parent walker.  

##### Setters
- <a name="carla.WalkerAIController.set_max_speed"></a>**<font color="#7fb800">set_max_speed</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**speed**=1.4</font>)  
Sets a speed for the walker in meters per second.  
    - **Parameters:**
        - `speed` (_float<small> - m/s</small>_) - An easy walking speed is set by default.  

##### Dunder methods
- <a name="carla.WalkerAIController.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerBoneControlIn<a name="carla.WalkerBoneControlIn"></a>
This class grants bone specific manipulation for walker. The skeletons of walkers have been unified for clarity and the transform applied to each bone are always relative to its parent. Take a look [here](tuto_G_control_walker_skeletons.md) to learn more on how to create a walker and define its movement.  

### Instance Variables
- <a name="carla.WalkerBoneControlIn.bone_transforms"></a>**<font color="#f8805a">bone_transforms</font>** (_list([name,transform])_)  
List with the data for each bone we want to set:
  - name: bone name
  - relative: transform based on the bone parent.  

### Methods
- <a name="carla.WalkerBoneControlIn.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**list(name,transform)**</font>)  
Initializes an object containing moves to be applied on tick. These are listed with the name of the bone and the transform that will be applied to it.  
    - **Parameters:**
        - `list(name,transform)` (_tuple_)  

##### Dunder methods
- <a name="carla.WalkerBoneControlIn.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerBoneControlOut<a name="carla.WalkerBoneControlOut"></a>
This class is used to return all bone positions of a pedestrian. For each bone we get its _name_ and its transform in three different spaces (world, actor and relative).  

### Instance Variables
- <a name="carla.WalkerBoneControlOut.bone_transforms"></a>**<font color="#f8805a">bone_transforms</font>** (_list([name,world, actor, relative])_)  
List of one entry per bone with this information:
  - name: bone name
  - world: transform in world coordinates
  - component: transform based on the pivot of the actor
  - relative: transform based on the bone parent.  

### Methods

##### Dunder methods
- <a name="carla.WalkerBoneControlOut.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerControl<a name="carla.WalkerControl"></a>
This class defines specific directions that can be commanded to a [carla.Walker](#carla.Walker) to control it via script.
  
  AI control can be settled for walkers, but the control used to do so is [carla.WalkerAIController](#carla.WalkerAIController).  

### Instance Variables
- <a name="carla.WalkerControl.direction"></a>**<font color="#f8805a">direction</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Vector using global coordinates that will correspond to the direction of the walker.  
- <a name="carla.WalkerControl.speed"></a>**<font color="#f8805a">speed</font>** (_float<small> - m/s</small>_)  
A scalar value to control the walker's speed.  
- <a name="carla.WalkerControl.jump"></a>**<font color="#f8805a">jump</font>** (_bool_)  
If <b>True</b>, the walker will perform a jump.  

### Methods
- <a name="carla.WalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**direction**=[1.0, 0.0, 0.0]</font>, <font color="#00a6ed">**speed**=0.0</font>, <font color="#00a6ed">**jump**=False</font>)  
    - **Parameters:**
        - `direction` (_[carla.Vector3D](#carla.Vector3D)_)  
        - `speed` (_float<small> - m/s</small>_)  
        - `jump` (_bool_)  

##### Dunder methods
- <a name="carla.WalkerControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WalkerControl](#carla.WalkerControl)</font>)  
Compares every variable with `other` and returns <b>True</b> if these are all the same.  
- <a name="carla.WalkerControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WalkerControl](#carla.WalkerControl)</font>)  
Compares every variable with `other` and returns <b>True</b> if any of these differ.  
- <a name="carla.WalkerControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Waypoint<a name="carla.Waypoint"></a>
Waypoints in CARLA are described as 3D directed points. They have a [carla.Transform](#carla.Transform) which locates the waypoint in a road and orientates it according to the lane. They also store the road information belonging to said point regarding its lane and lane markings.   <br><br> All the information regarding waypoints and the [waypoint API](../../core_map/#navigation-in-carla) is retrieved as provided by the OpenDRIVE file. Once the client asks for the map object to the server, no longer communication will be needed.  

### Instance Variables
- <a name="carla.Waypoint.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The identifier is generated using a hash combination of the <b>road</b>, <b>section</b>, <b>lane</b> and <b>s</b> values that correspond to said point in the OpenDRIVE geometry. The <b>s</b> precision is set to 2 centimeters, so 2 waypoints closer than 2 centimeters in the same road, section and lane, will have the same identificator.  
- <a name="carla.Waypoint.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Position and orientation of the waypoint according to the current lane information. This data is computed the first time it is accessed. It is not created right away in order to ease computing costs when lots of waypoints are created but their specific transform is not needed.  
- <a name="carla.Waypoint.road_id"></a>**<font color="#f8805a">road_id</font>** (_int_)  
OpenDRIVE road's id.  
- <a name="carla.Waypoint.section_id"></a>**<font color="#f8805a">section_id</font>** (_int_)  
OpenDRIVE section's id, based on the order that they are originally defined.  
- <a name="carla.Waypoint.is_junction"></a>**<font color="#f8805a">is_junction</font>** (_bool_)  
<b>True</b> if the current Waypoint is on a junction as defined by OpenDRIVE.  
- <a name="carla.Waypoint.junction_id"></a>**<font color="#f8805a">junction_id</font>** (_int_)  
OpenDRIVE junction's id. For more information refer to OpenDRIVE [documentation](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf#page=20).  
- <a name="carla.Waypoint.lane_id"></a>**<font color="#f8805a">lane_id</font>** (_int_)  
OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the current lane with respect to the road. For more information refer to OpenDRIVE [documentation](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf#page=20).  
- <a name="carla.Waypoint.s"></a>**<font color="#f8805a">s</font>** (_float_)  
OpenDRIVE <b>s</b> value of the current position.  
- <a name="carla.Waypoint.lane_width"></a>**<font color="#f8805a">lane_width</font>** (_float_)  
Horizontal size of the road at current <b>s</b>.  
- <a name="carla.Waypoint.lane_change"></a>**<font color="#f8805a">lane_change</font>** (_[carla.LaneChange](#carla.LaneChange)_)  
Lane change definition of the current Waypoint's location, based on the traffic rules defined in the OpenDRIVE file. It states if a lane change can be done and in which direction.  
- <a name="carla.Waypoint.lane_type"></a>**<font color="#f8805a">lane_type</font>** (_[carla.LaneType](#carla.LaneType)_)  
The lane type of the current Waypoint, based on OpenDRIVE 1.4 standard.  
- <a name="carla.Waypoint.right_lane_marking"></a>**<font color="#f8805a">right_lane_marking</font>** (_[carla.LaneMarking](#carla.LaneMarking)_)  
The right lane marking information based on the direction of the Waypoint.  
- <a name="carla.Waypoint.left_lane_marking"></a>**<font color="#f8805a">left_lane_marking</font>** (_[carla.LaneMarking](#carla.LaneMarking)_)  
The left lane marking information based on the direction of the Waypoint.  

### Methods
- <a name="carla.Waypoint.next"></a>**<font color="#7fb800">next</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints at a certain approximate `distance` from the current one. It takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option.
The list may be empty if the lane is not connected to any other at the specified distance.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - The approximate distance where to get the next waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.next_until_lane_end"></a>**<font color="#7fb800">next_until_lane_end</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints from this to the end of the lane separated by a certain `distance`.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - The approximate distance between waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.previous"></a>**<font color="#7fb800">previous</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
This method does not return the waypoint previously visited by an actor, but a list of waypoints at an approximate `distance` but in the opposite direction of the lane. Similarly to **<font color="#7fb800">next()</font>**, it takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option.
The list may be empty if the lane is not connected to any other at the specified distance.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - The approximate distance where to get the previous waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.previous_until_lane_start"></a>**<font color="#7fb800">previous_until_lane_start</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints from this to the start of the lane separated by a certain `distance`.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - The approximate distance between waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  

##### Getters
- <a name="carla.Waypoint.get_junction"></a>**<font color="#7fb800">get_junction</font>**(<font color="#00a6ed">**self**</font>)  
If the waypoint belongs to a junction this method returns the associated junction object. Otherwise returns null.  
    - **Return:** _[carla.Junction](#carla.Junction)_  
- <a name="carla.Waypoint.get_landmarks"></a>**<font color="#7fb800">get_landmarks</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>, <font color="#00a6ed">**stop_at_junction**=False</font>)  
Returns a list of landmarks in the road from the current waypoint until the specified distance.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - The maximum distance to search for landmarks from the current waypoint.  
        - `stop_at_junction` (_bool_) - Enables or disables the landmark search through junctions.  
    - **Return:** _list([carla.Landmark](#carla.Landmark))_  
- <a name="carla.Waypoint.get_landmarks_of_type"></a>**<font color="#7fb800">get_landmarks_of_type</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>, <font color="#00a6ed">**type**</font>, <font color="#00a6ed">**stop_at_junction**=False</font>)  
Returns a list of landmarks in the road of a specified type from the current waypoint until the specified distance.  
    - **Parameters:**
        - `distance` (_float<small> - meters</small>_) - The maximum distance to search for landmarks from the current waypoint.  
        - `type` (_str_) - The type of landmarks to search.  
        - `stop_at_junction` (_bool_) - Enables or disables the landmark search through junctions.  
    - **Return:** _list([carla.Landmark](#carla.Landmark))_  
- <a name="carla.Waypoint.get_left_lane"></a>**<font color="#7fb800">get_left_lane</font>**(<font color="#00a6ed">**self**</font>)  
Generates a Waypoint at the center of the left lane based on the direction of the current Waypoint, taking into account if the lane change is allowed in this location.
Will return <b>None</b> if the lane does not exist.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Waypoint.get_right_lane"></a>**<font color="#7fb800">get_right_lane</font>**(<font color="#00a6ed">**self**</font>)  
Generates a waypoint at the center of the right lane based on the direction of the current waypoint, taking into account if the lane change is allowed in this location.
Will return <b>None</b> if the lane does not exist.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  

##### Dunder methods
- <a name="carla.Waypoint.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WeatherParameters<a name="carla.WeatherParameters"></a>
This class defines objects containing lighting and weather specifications that can later be applied in [carla.World](#carla.World). So far, these conditions only intervene with [sensor.camera.rgb](ref_sensors.md#rgb-camera). They neither affect the actor's physics nor other sensors.        
  Each of these parameters acts indepently from the rest. Increasing the rainfall will not automatically create puddles nor change the road's humidity. That makes for a better customization but means that realistic conditions need to be scripted. However an example of dynamic weather conditions working realistically can be found [here](https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/dynamic_weather.py).  

### Instance Variables
- <a name="carla.WeatherParameters.cloudiness"></a>**<font color="#f8805a">cloudiness</font>** (_float_)  
Values range from 0 to 100, being 0 a clear sky and 100 one completely covered with clouds.  
- <a name="carla.WeatherParameters.precipitation"></a>**<font color="#f8805a">precipitation</font>** (_float_)  
Rain intensity values range from 0 to 100, being 0 none at all and 100 a heavy rain.  
- <a name="carla.WeatherParameters.precipitation_deposits"></a>**<font color="#f8805a">precipitation_deposits</font>** (_float_)  
Determines the creation of puddles. Values range from 0 to 100, being 0 none at all and 100 a road completely capped with water. Puddles are created with static noise, meaning that they will always appear at the same locations.  
- <a name="carla.WeatherParameters.wind_intensity"></a>**<font color="#f8805a">wind_intensity</font>** (_float_)  
Controls the strenght of the wind with values from 0, no wind at all, to 100, a strong wind. The wind does affect rain direction and leaves from trees, so this value is restricted to avoid animation issues.  
- <a name="carla.WeatherParameters.sun_azimuth_angle"></a>**<font color="#f8805a">sun_azimuth_angle</font>** (_float<small> - degrees</small>_)  
The azimuth angle of the sun. Values range from 0 to 360. Zero is an origin point in a sphere determined by Unreal Engine.  
- <a name="carla.WeatherParameters.sun_altitude_angle"></a>**<font color="#f8805a">sun_altitude_angle</font>** (_float<small> - degrees</small>_)  
Altitude angle of the sun. Values range from -90 to 90 corresponding to midnight and midday each.  
- <a name="carla.WeatherParameters.fog_density"></a>**<font color="#f8805a">fog_density</font>** (_float_)  
Fog concentration or thickness. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.fog_distance"></a>**<font color="#f8805a">fog_distance</font>** (_float<small> - meters</small>_)  
Fog start distance. Values range from 0 to infinite.  
- <a name="carla.WeatherParameters.wetness"></a>**<font color="#f8805a">wetness</font>** (_float_)  
Wetness intensity. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.fog_falloff"></a>**<font color="#f8805a">fog_falloff</font>** (_float_)  
Density of the fog (as in specific mass) from 0 to infinity. The bigger the value, the more dense and heavy it will be, and the fog will reach smaller heights. Corresponds to <a href="https://docs.unrealengine.com/en-US/Engine/Actors/FogEffects/HeightFog/index.html#:~:text=Using%20Exponential%20Height%20Fog%20Features,-The%20sections%20below&text=Add%20a%20second%20fog%20layer,height%20falloff%2C%20and%20height%20offset">Fog Height Falloff</a> in the UE docs. <br> If the value is 0, the fog will be lighter than air, and will cover the whole scene. <br> A value of 1 is approximately as dense as the air, and reaches normal-sized buildings. <br> For values greater than 5, the air will be so dense that it will be compressed on ground level.  
- <a name="carla.WeatherParameters.scattering_intensity"></a>**<font color="#f8805a">scattering_intensity</font>** (_float_)  
Controls how much the light will contribute to volumetric fog. When set to 0, there is no contribution.  
- <a name="carla.WeatherParameters.mie_scattering_scale"></a>**<font color="#f8805a">mie_scattering_scale</font>** (_float_)  
Controls interaction of light with large particles like pollen or air pollution resulting in a hazy sky with halos around the light sources. When set to 0, there is no contribution.  
- <a name="carla.WeatherParameters.rayleigh_scattering_scale"></a>**<font color="#f8805a">rayleigh_scattering_scale</font>** (_float_)  
Controls interaction of light with small particles like air molecules. Dependent on light wavelength, resulting in a blue sky in the day or red sky in the evening.  
- <a name="carla.WeatherParameters.dust_storm"></a>**<font color="#f8805a">dust_storm</font>** (_float_)  
Determines the strength of the dust storm weather. Values range from 0 to 100.  

### Methods
- <a name="carla.WeatherParameters.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**cloudiness**=0.0</font>, <font color="#00a6ed">**precipitation**=0.0</font>, <font color="#00a6ed">**precipitation_deposits**=0.0</font>, <font color="#00a6ed">**wind_intensity**=0.0</font>, <font color="#00a6ed">**sun_azimuth_angle**=0.0</font>, <font color="#00a6ed">**sun_altitude_angle**=0.0</font>, <font color="#00a6ed">**fog_density**=0.0</font>, <font color="#00a6ed">**fog_distance**=0.0</font>, <font color="#00a6ed">**wetness**=0.0</font>, <font color="#00a6ed">**fog_falloff**=0.0</font>, <font color="#00a6ed">**scattering_intensity**=0.0</font>, <font color="#00a6ed">**mie_scattering_scale**=0.0</font>, <font color="#00a6ed">**rayleigh_scattering_scale**=0.0331</font>)  
Method to initialize an object defining weather conditions. This class has some presets for different noon and sunset conditions listed in a note below.  
    - **Parameters:**
        - `cloudiness` (_float_) - 0 is a clear sky, 100 complete overcast.  
        - `precipitation` (_float_) - 0 is no rain at all, 100 a heavy rain.  
        - `precipitation_deposits` (_float_) - 0 means no puddles on the road, 100 means roads completely capped by rain.  
        - `wind_intensity` (_float_) - 0 is calm, 100 a strong wind.  
        - `sun_azimuth_angle` (_float<small> - degrees</small>_) - 0 is an arbitrary North, 180 its corresponding South.  
        - `sun_altitude_angle` (_float<small> - degrees</small>_) - 90 is midday, -90 is midnight.  
        - `fog_density` (_float_) - Concentration or thickness of the fog, from 0 to 100.  
        - `fog_distance` (_float<small> - meters</small>_) - Distance where the fog starts in meters.  
        - `wetness` (_float_) - Humidity percentages of the road, from 0 to 100.  
        - `fog_falloff` (_float_) - Density (specific mass) of the fog, from 0 to infinity.  
        - `scattering_intensity` (_float_) - Controls how much the light will contribute to volumetric fog. When set to 0, there is no contribution.  
        - `mie_scattering_scale` (_float_) - Controls interaction of light with large particles like pollen or air pollution resulting in a hazy sky with halos around the light sources. When set to 0, there is no contribution.  
        - `rayleigh_scattering_scale` (_float_) - Controls interaction of light with small particles like air molecules. Dependent on light wavelength, resulting in a blue sky in the day or red sky in the evening.  
    - **Note:** <font color="#8E8E8E">_ClearNoon, CloudyNoon, WetNoon, WetCloudyNoon, SoftRainNoon, MidRainyNoon, HardRainNoon, ClearSunset, CloudySunset, WetSunset, WetCloudySunset, SoftRainSunset, MidRainSunset, HardRainSunset. 
_</font>  

##### Dunder methods
- <a name="carla.WeatherParameters.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
Returns <b>True</b> if both objects' variables are the same.  
    - **Return:** _bool_  
- <a name="carla.WeatherParameters.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
Returns <b>True</b> if both objects' variables are different.  
    - **Return:** _bool_  
- <a name="carla.WeatherParameters.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WheelPhysicsControl<a name="carla.WheelPhysicsControl"></a>
Class that defines specific physical parameters for wheel objects that will be part of a [carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl) to simulate vehicle it as a material object.  

### Instance Variables
- <a name="carla.WheelPhysicsControl.tire_friction"></a>**<font color="#f8805a">tire_friction</font>** (_float_)  
A scalar value that indicates the friction of the wheel.  
- <a name="carla.WheelPhysicsControl.damping_rate"></a>**<font color="#f8805a">damping_rate</font>** (_float_)  
Damping rate of the wheel.  
- <a name="carla.WheelPhysicsControl.max_steer_angle"></a>**<font color="#f8805a">max_steer_angle</font>** (_float<small> - degrees</small>_)  
Maximum angle that the wheel can steer.  
- <a name="carla.WheelPhysicsControl.radius"></a>**<font color="#f8805a">radius</font>** (_float<small> - centimeters</small>_)  
Radius of the wheel.  
- <a name="carla.WheelPhysicsControl.max_brake_torque"></a>**<font color="#f8805a">max_brake_torque</font>** (_float<small> - N*m</small>_)  
Maximum brake torque.  
- <a name="carla.WheelPhysicsControl.max_handbrake_torque"></a>**<font color="#f8805a">max_handbrake_torque</font>** (_float<small> - N*m</small>_)  
Maximum handbrake torque.  
- <a name="carla.WheelPhysicsControl.position"></a>**<font color="#f8805a">position</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
World position of the wheel. This is a read-only parameter.  
- <a name="carla.WheelPhysicsControl.long_stiff_value"></a>**<font color="#f8805a">long_stiff_value</font>** (_float<small> - kg per radian</small>_)  
Tire longitudinal stiffness per unit gravitational acceleration. Each vehicle has a custom value.  
- <a name="carla.WheelPhysicsControl.lat_stiff_max_load"></a>**<font color="#f8805a">lat_stiff_max_load</font>** (_float_)  
Maximum normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire. Each vehicle has a custom value.  
- <a name="carla.WheelPhysicsControl.lat_stiff_value"></a>**<font color="#f8805a">lat_stiff_value</font>** (_float_)  
Maximum stiffness per unit of lateral slip. Each vehicle has a custom value.  

### Methods
- <a name="carla.WheelPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**tire_friction**=2.0</font>, <font color="#00a6ed">**damping_rate**=0.25</font>, <font color="#00a6ed">**max_steer_angle**=70.0</font>, <font color="#00a6ed">**radius**=30.0</font>, <font color="#00a6ed">**max_brake_torque**=1500.0</font>, <font color="#00a6ed">**max_handbrake_torque**=3000.0</font>, <font color="#00a6ed">**position**=(0.0,0.0,0.0)</font>)  
    - **Parameters:**
        - `tire_friction` (_float_)  
        - `damping_rate` (_float_)  
        - `max_steer_angle` (_float<small> - degrees</small>_)  
        - `radius` (_float<small> - centimerers</small>_)  
        - `max_brake_torque` (_float<small> - N*m</small>_)  
        - `max_handbrake_torque` (_float<small> - N*m</small>_)  
        - `position` (_[carla.Vector3D](#carla.Vector3D)<small> - meters</small>_)  

##### Dunder methods
- <a name="carla.WheelPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WheelPhysicsControl](#carla.WheelPhysicsControl)</font>)  
- <a name="carla.WheelPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WheelPhysicsControl](#carla.WheelPhysicsControl)</font>)  
- <a name="carla.WheelPhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.World<a name="carla.World"></a>
World objects are created by the client to have a place for the simulation to happen. The world contains the map we can see, meaning the asset, not the navigation map. Navigation maps are part of the [carla.Map](#carla.Map) class. It also manages the weather and actors present in it. There can only be one world per simulation, but it can be changed anytime.  

### Instance Variables
- <a name="carla.World.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The ID of the episode associated with this world. Episodes are different sessions of a simulation. These change everytime a world is disabled or reloaded. Keeping track is useful to avoid possible issues.  
- <a name="carla.World.debug"></a>**<font color="#f8805a">debug</font>** (_[carla.DebugHelper](#carla.DebugHelper)_)  
Responsible for creating different shapes for debugging. Take a look at its class to learn more about it.  

### Methods
- <a name="carla.World.apply_color_texture_to_object"></a>**<font color="#7fb800">apply_color_texture_to_object</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**object_name**</font>, <font color="#00a6ed">**material_parameter**</font>, <font color="#00a6ed">**texture**</font>)  
Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to `object_name`.  
    - **Parameters:**
        - `object_name` (_str_)  
        - `material_parameter` (_[carla.MaterialParameter](#carla.MaterialParameter)_)  
        - `texture` (_TextureColor_)  
- <a name="carla.World.apply_color_texture_to_objects"></a>**<font color="#7fb800">apply_color_texture_to_objects</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**objects_name_list**</font>, <font color="#00a6ed">**material_parameter**</font>, <font color="#00a6ed">**texture**</font>)  
Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to all objects in `objects_name_list`.  
    - **Parameters:**
        - `objects_name_list` (_list(str)_)  
        - `material_parameter` (_[carla.MaterialParameter](#carla.MaterialParameter)_)  
        - `texture` (_TextureColor_)  
- <a name="carla.World.apply_float_color_texture_to_object"></a>**<font color="#7fb800">apply_float_color_texture_to_object</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**object_name**</font>, <font color="#00a6ed">**material_parameter**</font>, <font color="#00a6ed">**texture**</font>)  
Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to `object_name`.  
    - **Parameters:**
        - `object_name` (_str_)  
        - `material_parameter` (_[carla.MaterialParameter](#carla.MaterialParameter)_)  
        - `texture` (_TextureFloatColor_)  
- <a name="carla.World.apply_float_color_texture_to_objects"></a>**<font color="#7fb800">apply_float_color_texture_to_objects</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**objects_name_list**</font>, <font color="#00a6ed">**material_parameter**</font>, <font color="#00a6ed">**texture**</font>)  
Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to all objects in `objects_name_list`.  
    - **Parameters:**
        - `objects_name_list` (_list(str)_)  
        - `material_parameter` (_[carla.MaterialParameter](#carla.MaterialParameter)_)  
        - `texture` (_TextureFloatColor_)  
- <a name="carla.World.apply_settings"></a>**<font color="#7fb800">apply_settings</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**world_settings**</font>)  
This method applies settings contained in an object to the simulation running and returns the ID of the frame they were implemented.  
    - **Parameters:**
        - `world_settings` (_[carla.WorldSettings](#carla.WorldSettings)_)  
    - **Return:** _int_  
    - **Warning:** <font color="#ED2F2F">_If synchronous mode is enabled, and there is a Traffic Manager running, this must be set to sync mode too. Read [this](adv_traffic_manager.md#synchronous-mode) to learn how to do it. 
_</font>  
- <a name="carla.World.apply_textures_to_object"></a>**<font color="#7fb800">apply_textures_to_object</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**object_name**</font>, <font color="#00a6ed">**diffuse_texture**</font>, <font color="#00a6ed">**emissive_texture**</font>, <font color="#00a6ed">**normal_texture**</font>, <font color="#00a6ed">**ao_roughness_metallic_emissive_texture**</font>)  
Applies all texture fields in [carla.MaterialParameter](#carla.MaterialParameter) to the object `object_name`. Empty textures here will not be applied.  
    - **Parameters:**
        - `object_name` (_str_)  
        - `diffuse_texture` (_TextureColor_)  
        - `emissive_texture` (_TextureFloatColor_)  
        - `normal_texture` (_TextureFloatColor_)  
        - `ao_roughness_metallic_emissive_texture` (_TextureFloatColor_)  
- <a name="carla.World.apply_textures_to_objects"></a>**<font color="#7fb800">apply_textures_to_objects</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**objects_name_list**</font>, <font color="#00a6ed">**diffuse_texture**</font>, <font color="#00a6ed">**emissive_texture**</font>, <font color="#00a6ed">**normal_texture**</font>, <font color="#00a6ed">**ao_roughness_metallic_emissive_texture**</font>)  
Applies all texture fields in [carla.MaterialParameter](#carla.MaterialParameter) to all objects in `objects_name_list`. Empty textures here will not be applied.  
    - **Parameters:**
        - `objects_name_list` (_list(str)_)  
        - `diffuse_texture` (_TextureColor_)  
        - `emissive_texture` (_TextureFloatColor_)  
        - `normal_texture` (_TextureFloatColor_)  
        - `ao_roughness_metallic_emissive_texture` (_TextureFloatColor_)  
- <a name="carla.World.cast_ray"></a>**<font color="#7fb800">cast_ray</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**initial_location**</font>, <font color="#00a6ed">**final_location**</font>)  
Casts a ray from the specified initial_location to final_location. The function then detects all geometries intersecting the ray and returns a list of [carla.LabelledPoint](#carla.LabelledPoint) in order.  
    - **Parameters:**
        - `initial_location` (_[carla.Location](#carla.Location)_) - The initial position of the ray.  
        - `final_location` (_[carla.Location](#carla.Location)_) - The final position of the ray.  
    - **Return:** _list([carla.LabelledPoint](#carla.LabelledPoint))_  
- <a name="carla.World.enable_environment_objects"></a>**<font color="#7fb800">enable_environment_objects</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**env_objects_ids**</font>, <font color="#00a6ed">**enable**</font>)<button class="SnipetButton" id="carla.World.enable_environment_objects-snipet_button">snippet &rarr;</button>  
Enable or disable a set of EnvironmentObject identified by their id. These objects will appear or disappear from the level.  
    - **Parameters:**
        - `env_objects_ids` (_set(int)_) - Set of EnvironmentObject ids to change.  
        - `enable` (_bool_) - State to be applied to all the EnvironmentObject of the set.  
- <a name="carla.World.freeze_all_traffic_lights"></a>**<font color="#7fb800">freeze_all_traffic_lights</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**frozen**</font>)  
Freezes or unfreezes all traffic lights in the scene. Frozen traffic lights can be modified by the user but the time will not update them until unfrozen.  
    - **Parameters:**
        - `frozen` (_bool_)  
- <a name="carla.World.ground_projection"></a>**<font color="#7fb800">ground_projection</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**search_distance**</font>)  
Projects the specified point downwards in the scene. The functions casts a ray from location in the direction (0,0,-1) (downwards) and returns a [carla.LabelledPoint](#carla.LabelledPoint) object with the first geometry this ray intersects (usually the ground). If no geometry is found in the search_distance range the function returns `None`.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) - The point to be projected.  
        - `search_distance` (_float_) - The maximum distance to perform the projection.  
    - **Return:** _[carla.LabelledPoint](#carla.LabelledPoint)_  
- <a name="carla.World.load_map_layer"></a>**<font color="#7fb800">load_map_layer</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**map_layers**</font>)<button class="SnipetButton" id="carla.World.load_map_layer-snipet_button">snippet &rarr;</button>  
Loads the selected layers to the level. If the layer is already loaded the call has no effect.  
    - **Parameters:**
        - `map_layers` (_[carla.MapLayer](#carla.MapLayer)_) - Mask of level layers to be loaded.  
    - **Warning:** <font color="#ED2F2F">_This only affects "Opt" maps. The minimum layout includes roads, sidewalks, traffic lights and traffic signs._</font>  
- <a name="carla.World.on_tick"></a>**<font color="#7fb800">on_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)  
This method is used in [__asynchronous__ mode](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/adv_synchrony_timestep/). It starts callbacks from the client for the function defined as `callback`, and returns the ID of the callback. The function will be called everytime the server ticks. It requires a [carla.WorldSnapshot](#carla.WorldSnapshot) as argument, which can be retrieved from __<font color="#7fb800">wait_for_tick()</font>__. Use __<font color="#7fb800">remove_on_tick()</font>__ to stop the callbacks.  
    - **Parameters:**
        - `callback` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_) - Function with a snapshot as compulsory parameter that will be called when the client receives a tick.  
    - **Return:** _int_  
- <a name="carla.World.project_point"></a>**<font color="#7fb800">project_point</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**direction**</font>, <font color="#00a6ed">**search_distance**</font>)  
Projects the specified point to the desired direction in the scene. The functions casts a ray from location in a direction and returns a [carla.Labelled](#carla.Labelled) object with the first geometry this ray intersects. If no geometry is found in the search_distance range the function returns `None`.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) - The point to be projected.  
        - `direction` (_[carla.Vector3D](#carla.Vector3D)_) - The direction of projection.  
        - `search_distance` (_float_) - The maximum distance to perform the projection.  
    - **Return:** _[carla.LabelledPoint](#carla.LabelledPoint)_  
- <a name="carla.World.remove_on_tick"></a>**<font color="#7fb800">remove_on_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback_id**</font>)  
Stops the callback for `callback_id` started with __<font color="#7fb800">on_tick()</font>__.  
    - **Parameters:**
        - `callback_id` (_callback_) - The callback to be removed. The ID is returned when creating the callback.  
- <a name="carla.World.reset_all_traffic_lights"></a>**<font color="#7fb800">reset_all_traffic_lights</font>**(<font color="#00a6ed">**self**</font>)  
Resets the cycle of all traffic lights in the map to the initial state.  
- <a name="carla.World.spawn_actor"></a>**<font color="#7fb800">spawn_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**attach_to**=None</font>, <font color="#00a6ed">**attachment**=Rigid</font>)<button class="SnipetButton" id="carla.World.spawn_actor-snipet_button">snippet &rarr;</button>  
The method will create, return and spawn an actor into the world. The actor will need an available blueprint to be created and a transform (location and rotation). It can also be attached to a parent with a certain attachment type.  
    - **Parameters:**
        - `blueprint` (_[carla.ActorBlueprint](#carla.ActorBlueprint)_) - The reference from which the actor will be created.  
        - `transform` (_[carla.Transform](#carla.Transform)_) - Contains the location and orientation the actor will be spawned with.  
        - `attach_to` (_[carla.Actor](#carla.Actor)_) - The parent object that the spawned actor will follow around.  
        - `attachment` (_[carla.AttachmentType](#carla.AttachmentType)_) - Determines how fixed and rigorous should be the changes in position according to its parent object.  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.tick"></a>**<font color="#7fb800">tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**=10.0</font>)  
This method is used in [__synchronous__ mode](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/adv_synchrony_timestep/), when the server waits for a client tick before computing the next frame. This method will send the tick, and give way to the server. It returns the ID of the new frame computed by the server.  
    - **Parameters:**
        - `seconds` (_float<small> - seconds</small>_) - Maximum time the server should wait for a tick. It is set to <code>10.0</code> by default.  
    - **Return:** _int_  
    - **Note:** <font color="#8E8E8E">_If no tick is received in synchronous mode, the simulation will freeze. Also, if many ticks are received from different clients, there may be synchronization issues. Please read the docs about [synchronous mode](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/adv_synchrony_timestep/) to learn more.  
_</font>  
- <a name="carla.World.try_spawn_actor"></a>**<font color="#7fb800">try_spawn_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**attach_to**=None</font>, <font color="#00a6ed">**attachment**=Rigid</font>)  
Same as __<font color="#7fb800">spawn_actor()</font>__ but returns <b>None</b> on failure instead of throwing an exception.  
    - **Parameters:**
        - `blueprint` (_[carla.ActorBlueprint](#carla.ActorBlueprint)_) - The reference from which the actor will be created.  
        - `transform` (_[carla.Transform](#carla.Transform)_) - Contains the location and orientation the actor will be spawned with.  
        - `attach_to` (_[carla.Actor](#carla.Actor)_) - The parent object that the spawned actor will follow around.  
        - `attachment` (_[carla.AttachmentType](#carla.AttachmentType)_) - Determines how fixed and rigorous should be the changes in position according to its parent object.  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.unload_map_layer"></a>**<font color="#7fb800">unload_map_layer</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**map_layers**</font>)<button class="SnipetButton" id="carla.World.unload_map_layer-snipet_button">snippet &rarr;</button>  
Unloads the selected layers to the level. If the layer is already unloaded the call has no effect.  
    - **Parameters:**
        - `map_layers` (_[carla.MapLayer](#carla.MapLayer)_) - Mask of level layers to be unloaded.  
    - **Warning:** <font color="#ED2F2F">_This only affects "Opt" maps. The minimum layout includes roads, sidewalks, traffic lights and traffic signs._</font>  
- <a name="carla.World.wait_for_tick"></a>**<font color="#7fb800">wait_for_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**=10.0</font>)  
This method is used in [__asynchronous__ mode](https://[carla.readthedocs.io](#carla.readthedocs.io)/en/latest/adv_synchrony_timestep/). It makes the client wait for a server tick. When the next frame is computed, the server will tick and return a snapshot describing the new state of the world.  
    - **Parameters:**
        - `seconds` (_float<small> - seconds</small>_) - Maximum time the server should wait for a tick. It is set to <code>10.0</code> by default.  
    - **Return:** _[carla.WorldSnapshot](#carla.WorldSnapshot)_  

##### Getters
- <a name="carla.World.get_actor"></a>**<font color="#7fb800">get_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Looks up for an actor by ID and returns <b>None</b> if not found.  
    - **Parameters:**
        - `actor_id` (_int_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.get_actors"></a>**<font color="#7fb800">get_actors</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_ids**=None</font>)  
Retrieves a list of [carla.Actor](#carla.Actor) elements, either using a list of IDs provided or just listing everyone on stage. If an ID does not correspond with any actor, it will be excluded from the list returned, meaning that both the list of IDs and the list of actors may have different lengths.  
    - **Parameters:**
        - `actor_ids` (_list_) - The IDs of the actors being searched. By default it is set to <b>None</b> and returns every actor on scene.  
    - **Return:** _[carla.ActorList](#carla.ActorList)_  
- <a name="carla.World.get_blueprint_library"></a>**<font color="#7fb800">get_blueprint_library</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of actor blueprints available to ease the spawn of these into the world.  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
- <a name="carla.World.get_environment_objects"></a>**<font color="#7fb800">get_environment_objects</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**object_type**=Any</font>)  
Returns a list of EnvironmentObject with the requested semantic tag.  The method returns all the EnvironmentObjects in the level by default, but the query can be filtered by semantic tags with the argument `object_type`.  
    - **Parameters:**
        - `object_type` (_[carla.CityObjectLabel](#carla.CityObjectLabel)_) - Semantic tag of the EnvironmentObjects that are returned.  
    - **Return:** _array([carla.EnvironmentObject](#carla.EnvironmentObject))_  
- <a name="carla.World.get_level_bbs"></a>**<font color="#7fb800">get_level_bbs</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_type**=Any</font>)  
Returns an array of bounding boxes with location and rotation in world space. The method returns all the bounding boxes in the level by default, but the query can be filtered by semantic tags with the argument `actor_type`.  
    - **Parameters:**
        - `actor_type` (_[carla.CityObjectLabel](#carla.CityObjectLabel)_) - Semantic tag of the elements contained in the bounding boxes that are returned.  
    - **Return:** _array([carla.BoundingBox](#carla.BoundingBox))_  
- <a name="carla.World.get_lightmanager"></a>**<font color="#7fb800">get_lightmanager</font>**(<font color="#00a6ed">**self**</font>)  
Returns an instance of [carla.LightManager](#carla.LightManager) that can be used to handle the lights in the scene.  
    - **Return:** _[carla.LightManager](#carla.LightManager)_  
- <a name="carla.World.get_map"></a>**<font color="#7fb800">get_map</font>**(<font color="#00a6ed">**self**</font>)  
Asks the server for the XODR containing the map file, and returns this parsed as a [carla.Map](#carla.Map).  
    - **Return:** _[carla.Map](#carla.Map)_  
    - **Warning:** <font color="#ED2F2F">_This method does call the simulation. It is expensive, and should only be called once.  
_</font>  
- <a name="carla.World.get_names_of_all_objects"></a>**<font color="#7fb800">get_names_of_all_objects</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of the names of all objects in the scene that can be painted with the apply texture functions.  
    - **Return:** _list(str)_  
- <a name="carla.World.get_random_location_from_navigation"></a>**<font color="#7fb800">get_random_location_from_navigation</font>**(<font color="#00a6ed">**self**</font>)  
This can only be used with walkers. It retrieves a random location to be used as a destination using the __<font color="#7fb800">go_to_location()</font>__ method in [carla.WalkerAIController](#carla.WalkerAIController). This location will be part of a sidewalk. Roads, crosswalks and grass zones are excluded. The method does not take into consideration locations of existing actors so if a collision happens when trying to spawn an actor, it will return an error. Take a look at [`generate_traffic.py`](https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/generate_traffic.py) for an example.  
    - **Return:** _[carla.Location](#carla.Location)_  
- <a name="carla.World.get_settings"></a>**<font color="#7fb800">get_settings</font>**(<font color="#00a6ed">**self**</font>)  
Returns an object containing some data about the simulation such as synchrony between client and server or rendering mode.  
    - **Return:** _[carla.WorldSettings](#carla.WorldSettings)_  
- <a name="carla.World.get_snapshot"></a>**<font color="#7fb800">get_snapshot</font>**(<font color="#00a6ed">**self**</font>)  
Returns a snapshot of the world at a certain moment comprising all the information about the actors.  
    - **Return:** _[carla.WorldSnapshot](#carla.WorldSnapshot)_  
- <a name="carla.World.get_spectator"></a>**<font color="#7fb800">get_spectator</font>**(<font color="#00a6ed">**self**</font>)<button class="SnipetButton" id="carla.World.get_spectator-snipet_button">snippet &rarr;</button>  
Returns the spectator actor. The spectator is a special type of actor created by Unreal Engine, usually with ID=0, that acts as a camera and controls the view in the simulator window.  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.get_traffic_light"></a>**<font color="#7fb800">get_traffic_light</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**landmark**</font>)  
Provided a landmark, returns the traffic light object it describes.  
    - **Parameters:**
        - `landmark` (_[carla.Landmark](#carla.Landmark)_) - The landmark object describing a traffic light.  
    - **Return:** _[carla.TrafficLight](#carla.TrafficLight)_  
- <a name="carla.World.get_traffic_light_from_opendrive_id"></a>**<font color="#7fb800">get_traffic_light_from_opendrive_id</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**traffic_light_id**</font>)  
Returns the traffic light actor corresponding to the indicated OpenDRIVE id.  
    - **Parameters:**
        - `traffic_light_id` (_str_) - The OpenDRIVE id.  
    - **Return:** _[carla.TrafficLight](#carla.TrafficLight)_  
- <a name="carla.World.get_traffic_lights_from_waypoint"></a>**<font color="#7fb800">get_traffic_lights_from_waypoint</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**waypoint**</font>, <font color="#00a6ed">**distance**</font>)  
This function performs a search along the road in front of the specified waypoint and returns a list of traffic light actors found in the specified search distance.  
    - **Parameters:**
        - `waypoint` (_[carla.Waypoint](#carla.Waypoint)_) - The input waypoint.  
        - `distance` (_float_) - Search distance.  
    - **Return:** _list([carla.TrafficLight](#carla.TrafficLight))_  
- <a name="carla.World.get_traffic_lights_in_junction"></a>**<font color="#7fb800">get_traffic_lights_in_junction</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**junction_id**</font>)  
Returns the list of traffic light actors affecting the junction indicated in `junction_id`.  
    - **Parameters:**
        - `junction_id` (_int_) - The id of the junction.  
    - **Return:** _list([carla.TrafficLight](#carla.TrafficLight))_  
- <a name="carla.World.get_traffic_sign"></a>**<font color="#7fb800">get_traffic_sign</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**landmark**</font>)  
Provided a landmark, returns the traffic sign object it describes.  
    - **Parameters:**
        - `landmark` (_[carla.Landmark](#carla.Landmark)_) - The landmark object describing a traffic sign.  
    - **Return:** _[carla.TrafficSign](#carla.TrafficSign)_  
- <a name="carla.World.get_vehicles_light_states"></a>**<font color="#7fb800">get_vehicles_light_states</font>**(<font color="#00a6ed">**self**</font>)  
Returns a dict where the keys are [carla.Actor](#carla.Actor) IDs and the values are [carla.VehicleLightState](#carla.VehicleLightState) of that vehicle.  
    - **Return:** _dict_  
- <a name="carla.World.get_weather"></a>**<font color="#7fb800">get_weather</font>**(<font color="#00a6ed">**self**</font>)  
Retrieves an object containing weather parameters currently active in the simulation, mainly cloudiness, precipitation, wind and sun position.  
    - **Return:** _[carla.WeatherParameters](#carla.WeatherParameters)_  
    - **Setter:** _[carla.World.set_weather](#carla.World.set_weather)_  

##### Setters
- <a name="carla.World.set_pedestrians_cross_factor"></a>**<font color="#7fb800">set_pedestrians_cross_factor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**percentage**</font>)  
    - **Parameters:**
        - `percentage` (_float_) - Sets the percentage of pedestrians that can walk on the road or cross at any point on the road. Value should be between `0.0` and `1.0`. For example, a value of `0.1` would allow 10% of pedestrians to walk on the road. __Default is `0.0`__.  
    - **Note:** <font color="#8E8E8E">_Should be set before pedestrians are spawned.
_</font>  
- <a name="carla.World.set_pedestrians_seed"></a>**<font color="#7fb800">set_pedestrians_seed</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seed**</font>)  
    - **Parameters:**
        - `seed` (_int_) - Sets the seed to use for any random number generated in relation to pedestrians.  
    - **Note:** <font color="#8E8E8E">_Should be set before pedestrians are spawned. If you want to repeat the same exact bodies (blueprint) for each pedestrian, then use the same seed in the Python code (where the blueprint is choosen randomly) and here, otherwise the pedestrians will repeat the same paths but the bodies will be different.
_</font>  
- <a name="carla.World.set_weather"></a>**<font color="#7fb800">set_weather</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**weather**</font>)  
Changes the weather parameteres ruling the simulation to another ones defined in an object.  
    - **Parameters:**
        - `weather` (_[carla.WeatherParameters](#carla.WeatherParameters)_) - New conditions to be applied.  
    - **Getter:** _[carla.World.get_weather](#carla.World.get_weather)_  

##### Dunder methods
- <a name="carla.World.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
The content of the world is parsed and printed as a brief report of its current state.  
    - **Return:** _string_  

---

## carla.WorldSettings<a name="carla.WorldSettings"></a>
The simulation has some advanced configuration options that are contained in this class and can be managed using [carla.World](#carla.World) and its methods. These allow the user to choose between client-server synchrony/asynchrony, activation of "no rendering mode" and either if the simulation should run with a fixed or variable time-step. Check [this](adv_synchrony_timestep.md) out if you want to learn about it.  

### Instance Variables
- <a name="carla.WorldSettings.synchronous_mode"></a>**<font color="#f8805a">synchronous_mode</font>** (_bool_)  
States the synchrony between client and server. When set to true, the server will wait for a client tick in order to move forward. It is false by default.  
- <a name="carla.WorldSettings.no_rendering_mode"></a>**<font color="#f8805a">no_rendering_mode</font>** (_bool_)  
When enabled, the simulation will run no rendering at all. This is mainly used to avoid overhead during heavy traffic simulations. It is false by default.  
- <a name="carla.WorldSettings.fixed_delta_seconds"></a>**<font color="#f8805a">fixed_delta_seconds</font>** (_float_)  
Ensures that the time elapsed between two steps of the simulation is fixed. Set this to <b>0.0</b> to work with a variable time-step, as happens by default.  
- <a name="carla.WorldSettings.substepping"></a>**<font color="#f8805a">substepping</font>** (_bool_)  
Enable the physics substepping. This option allows computing some physics substeps between two render frames. If synchronous mode is set, the number of substeps and its time interval are fixed and computed are so they fulfilled the requirements of [carla.WorldSettings.max_substep](#carla.WorldSettings.max_substep) and [carla.WorldSettings.max_substep_delta_time](#carla.WorldSettings.max_substep_delta_time). These last two parameters need to be compatible with [carla.WorldSettings.fixed_delta_seconds](#carla.WorldSettings.fixed_delta_seconds). Enabled by default.  
- <a name="carla.WorldSettings.max_substep_delta_time"></a>**<font color="#f8805a">max_substep_delta_time</font>** (_float_)  
Maximum delta time of the substeps. If the [carla.WorldSettingsmax_substep](#carla.WorldSettingsmax_substep) is high enough, the substep delta time would be always below or equal to this value. By default, the value is set to 0.01.  
- <a name="carla.WorldSettings.max_substeps"></a>**<font color="#f8805a">max_substeps</font>** (_int_)  
The maximum number of physics substepping that are allowed. By default, the value is set to 10.  
- <a name="carla.WorldSettings.max_culling_distance"></a>**<font color="#f8805a">max_culling_distance</font>** (_float_)  
Configure the max draw distance for each mesh of the level.  
- <a name="carla.WorldSettings.deterministic_ragdolls"></a>**<font color="#f8805a">deterministic_ragdolls</font>** (_bool_)  
Defines wether to use deterministic physics for pedestrian death animations or physical ragdoll simulation.  When enabled, pedestrians have less realistic death animation but ensures determinism.  When disabled, pedestrians are simulated as ragdolls with more realistic simulation and collision but no determinsm can be ensured.  
- <a name="carla.WorldSettings.tile_stream_distance"></a>**<font color="#f8805a">tile_stream_distance</font>** (_float_)  
Used for large maps only. Configures the maximum distance from the hero vehicle to stream tiled maps. Regions of the map within this range will be visible (and capable of simulating physics). Regions outside this region will not be loaded.  
- <a name="carla.WorldSettings.actor_active_distance"></a>**<font color="#f8805a">actor_active_distance</font>** (_float_)  
Used for large maps only. Configures the distance from the hero vehicle to convert actors to dormant. Actors within this range will be active, and actors outside will become dormant.  
- <a name="carla.WorldSettings.spectator_as_ego"></a>**<font color="#f8805a">spectator_as_ego</font>** (_bool_)  
Used for large maps only. Defines the influence of the spectator on tile loading in Large Maps. By default, the spectator will provoke loading of neighboring tiles in the absence of an ego actor. This might be inconvenient for applications that immediately spawn an ego actor.  

### Methods
- <a name="carla.WorldSettings.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**synchronous_mode**=False</font>, <font color="#00a6ed">**no_rendering_mode**=False</font>, <font color="#00a6ed">**fixed_delta_seconds**=0.0</font>, <font color="#00a6ed">**max_culling_distance**=0.0</font>, <font color="#00a6ed">**deterministic_ragdolls**=False</font>, <font color="#00a6ed">**tile_stream_distance**=3000</font>, <font color="#00a6ed">**actor_active_distance**=2000</font>, <font color="#00a6ed">**spectator_as_ego**=True</font>)  
Creates an object containing desired settings that could later be applied through [carla.World](#carla.World) and its method __<font color="#7fb800">apply_settings()</font>__.  
    - **Parameters:**
        - `synchronous_mode` (_bool_) - Set this to true to enable client-server synchrony.  
        - `no_rendering_mode` (_bool_) - Set this to true to completely disable rendering in the simulation.  
        - `fixed_delta_seconds` (_float<small> - seconds</small>_) - Set a fixed time-step in between frames. <code>0.0</code> means variable time-step and it is the default mode.  
        - `max_culling_distance` (_float<small> - meters</small>_) - Configure the max draw distance for each mesh of the level.  
        - `deterministic_ragdolls` (_bool_) - Defines wether to use deterministic physics or ragdoll simulation for pedestrian deaths.  
        - `tile_stream_distance` (_float<small> - meters</small>_) - Used for large maps only. Configures the maximum distance from the hero vehicle to stream tiled maps.  
        - `actor_active_distance` (_float<small> - meters</small>_) - Used for large maps only. Configures the distance from the hero vehicle to convert actors to dormant.  
        - `spectator_as_ego` (_bool_) - Used for large maps only. Defines the influence of the spectator on tile loading in Large Maps.  

##### Dunder methods
- <a name="carla.WorldSettings.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WorldSettings](#carla.WorldSettings)</font>)  
Returns <b>True</b> if both objects' variables are the same.  
    - **Return:** _bool_  
- <a name="carla.WorldSettings.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WorldSettings](#carla.WorldSettings)</font>)  
Returns <b>True</b> if both objects' variables are different.  
    - **Return:** _bool_  
- <a name="carla.WorldSettings.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
Parses the established settings to a string and shows them in command line.  
    - **Return:** _str_  

---

## carla.WorldSnapshot<a name="carla.WorldSnapshot"></a>
This snapshot comprises all the information for every actor on scene at a certain moment of time. It creates and gives acces to a data structure containing a series of [carla.ActorSnapshot](#carla.ActorSnapshot). The client recieves a new snapshot on every tick that cannot be stored.  

### Instance Variables
- <a name="carla.WorldSnapshot.id"></a>**<font color="#f8805a">id</font>** (_int_)  
A value unique for every snapshot to differentiate them.  
- <a name="carla.WorldSnapshot.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
Simulation frame in which the snapshot was taken.  
- <a name="carla.WorldSnapshot.timestamp"></a>**<font color="#f8805a">timestamp</font>** (_[carla.Timestamp](#carla.Timestamp)<small> - seconds</small>_)  
Precise moment in time when snapshot was taken. This class works in seconds as given by the operative system.  

### Methods
- <a name="carla.WorldSnapshot.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Given a certain actor ID, returns its corresponding snapshot or <b>None</b> if it is not found.  
    - **Parameters:**
        - `actor_id` (_int_)  
    - **Return:** _[carla.ActorSnapshot](#carla.ActorSnapshot)_  
- <a name="carla.WorldSnapshot.has_actor"></a>**<font color="#7fb800">has_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Given a certain actor ID, checks if there is a snapshot corresponding it and so, if the actor was present at that moment.  
    - **Parameters:**
        - `actor_id` (_int_)  
    - **Return:** _bool_  

##### Dunder methods
- <a name="carla.WorldSnapshot.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WorldSnapshot](#carla.WorldSnapshot)</font>)  
Returns __True__ if both **<font color="#f8805a">timestamp</font>** are the same.  
    - **Return:** _bool_  
- <a name="carla.WorldSnapshot.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
Iterate over the [carla.ActorSnapshot](#carla.ActorSnapshot) stored in the snapshot.  
- <a name="carla.WorldSnapshot.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
Returns the amount of [carla.ActorSnapshot](#carla.ActorSnapshot) present in this snapshot.  
    - **Return:** _int_  
- <a name="carla.WorldSnapshot.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**=[carla.WorldSnapshot](#carla.WorldSnapshot)</font>)  
Returns <b>True</b> if both **<font color="#f8805a">timestamp</font>** are different.  
    - **Return:** _bool_  

---

## command.ApplyAngularImpulse<a name="command.ApplyAngularImpulse"></a>
Command adaptation of __<font color="#7fb800">add_angular_impulse()</font>__ in [carla.Actor](#carla.Actor). Applies an angular impulse to an actor.  

### Instance Variables
- <a name="command.ApplyAngularImpulse.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyAngularImpulse.impulse"></a>**<font color="#f8805a">impulse</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - degrees*s</small>_)  
Angular impulse applied to the actor.  

### Methods
- <a name="command.ApplyAngularImpulse.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**impulse**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)<small> - degrees*s</small>_)  

---

## command.ApplyForce<a name="command.ApplyForce"></a>
Command adaptation of __<font color="#7fb800">add_force()</font>__ in [carla.Actor](#carla.Actor). Applies a force to an actor.  

### Instance Variables
- <a name="command.ApplyForce.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyForce.force"></a>**<font color="#f8805a">force</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - N</small>_)  
Force applied to the actor over time.  

### Methods
- <a name="command.ApplyForce.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**force**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `force` (_[carla.Vector3D](#carla.Vector3D)<small> - N</small>_)  

---

## command.ApplyImpulse<a name="command.ApplyImpulse"></a>
Command adaptation of __<font color="#7fb800">add_impulse()</font>__ in [carla.Actor](#carla.Actor). Applies an impulse to an actor.  

### Instance Variables
- <a name="command.ApplyImpulse.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyImpulse.impulse"></a>**<font color="#f8805a">impulse</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - N*s</small>_)  
Impulse applied to the actor.  

### Methods
- <a name="command.ApplyImpulse.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**impulse**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)<small> - N*s</small>_)  

---

## command.ApplyTargetAngularVelocity<a name="command.ApplyTargetAngularVelocity"></a>
Command adaptation of __<font color="#7fb800">set_target_angular_velocity()</font>__ in [carla.Actor](#carla.Actor). Sets the actor's angular velocity vector.  

### Instance Variables
- <a name="command.ApplyTargetAngularVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyTargetAngularVelocity.angular_velocity"></a>**<font color="#f8805a">angular_velocity</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - deg/s</small>_)  
The 3D angular velocity that will be applied to the actor.  

### Methods
- <a name="command.ApplyTargetAngularVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**angular_velocity**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `angular_velocity` (_[carla.Vector3D](#carla.Vector3D)<small> - deg/s</small>_) - Angular velocity vector applied to the actor.  

---

## command.ApplyTargetVelocity<a name="command.ApplyTargetVelocity"></a>
Command adaptation of __<font color="#7fb800">set_target_velocity()</font>__ in [carla.Actor](#carla.Actor).  

### Instance Variables
- <a name="command.ApplyTargetVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyTargetVelocity.velocity"></a>**<font color="#f8805a">velocity</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - m/s</small>_)  
The 3D velocity applied to the actor.  

### Methods
- <a name="command.ApplyTargetVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**velocity**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)<small> - m/s</small>_) - Velocity vector applied to the actor.  

---

## command.ApplyTorque<a name="command.ApplyTorque"></a>
Command adaptation of __<font color="#7fb800">add_torque()</font>__ in [carla.Actor](#carla.Actor). Applies a torque to an actor.  

### Instance Variables
- <a name="command.ApplyTorque.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyTorque.torque"></a>**<font color="#f8805a">torque</font>** (_[carla.Vector3D](#carla.Vector3D)<small> - degrees</small>_)  
Torque applied to the actor over time.  

### Methods
- <a name="command.ApplyTorque.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**torque**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `torque` (_[carla.Vector3D](#carla.Vector3D)<small> - degrees</small>_)  

---

## command.ApplyTransform<a name="command.ApplyTransform"></a>
Command adaptation of __<font color="#7fb800">set_transform()</font>__ in [carla.Actor](#carla.Actor). Sets a new transform to an actor.  

### Instance Variables
- <a name="command.ApplyTransform.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyTransform.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transformation to be applied.  

### Methods
- <a name="command.ApplyTransform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**transform**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `transform` (_[carla.Transform](#carla.Transform)_)  

---

## command.ApplyVehicleAckermannControl<a name="command.ApplyVehicleAckermannControl"></a>
Command adaptation of __<font color="#7fb800">apply_ackermann_control()</font>__ in [carla.Vehicle](#carla.Vehicle). Applies a certain akermann control to a vehicle.  

### Instance Variables
- <a name="command.ApplyVehicleAckermannControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Vehicle actor affected by the command.  
- <a name="command.ApplyVehicleAckermannControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.AckermannVehicleControl](#carla.AckermannVehicleControl)_)  
Vehicle ackermann control to be applied.  

### Methods
- <a name="command.ApplyVehicleAckermannControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `control` (_[carla.AckermannVehicleControl](#carla.AckermannVehicleControl)_)  

---

## command.ApplyVehicleControl<a name="command.ApplyVehicleControl"></a>
Command adaptation of __<font color="#7fb800">apply_control()</font>__ in [carla.Vehicle](#carla.Vehicle). Applies a certain control to a vehicle.  

### Instance Variables
- <a name="command.ApplyVehicleControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Vehicle actor affected by the command.  
- <a name="command.ApplyVehicleControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.VehicleControl](#carla.VehicleControl)_)  
Vehicle control to be applied.  

### Methods
- <a name="command.ApplyVehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `control` (_[carla.VehicleControl](#carla.VehicleControl)_)  

---

## command.ApplyVehiclePhysicsControl<a name="command.ApplyVehiclePhysicsControl"></a>
Command adaptation of __<font color="#7fb800">apply_physics_control()</font>__ in [carla.Vehicle](#carla.Vehicle). Applies a new physics control to a vehicle, modifying its physical parameters.  

### Instance Variables
- <a name="command.ApplyVehiclePhysicsControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Vehicle actor affected by the command.  
- <a name="command.ApplyVehiclePhysicsControl.physics_control"></a>**<font color="#f8805a">physics_control</font>** (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
Physics control to be applied.  

### Methods
- <a name="command.ApplyVehiclePhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**physics_control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `physics_control` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  

---

## command.ApplyWalkerControl<a name="command.ApplyWalkerControl"></a>
Command adaptation of __<font color="#7fb800">apply_control()</font>__ in [carla.Walker](#carla.Walker). Applies a control to a walker.  

### Instance Variables
- <a name="command.ApplyWalkerControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Walker actor affected by the command.  
- <a name="command.ApplyWalkerControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.WalkerControl](#carla.WalkerControl)_)  
Walker control to be applied.  

### Methods
- <a name="command.ApplyWalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `control` (_[carla.WalkerControl](#carla.WalkerControl)_)  

---

## command.ApplyWalkerState<a name="command.ApplyWalkerState"></a>
Apply a state to the walker actor. Specially useful to initialize an actor them with a specific location, orientation and speed.  

### Instance Variables
- <a name="command.ApplyWalkerState.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Walker actor affected by the command.  
- <a name="command.ApplyWalkerState.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transform to be applied.  
- <a name="command.ApplyWalkerState.speed"></a>**<font color="#f8805a">speed</font>** (_float<small> - m/s</small>_)  
Speed to be applied.  

### Methods
- <a name="command.ApplyWalkerState.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**speed**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
        - `speed` (_float<small> - m/s</small>_)  

---

## command.DestroyActor<a name="command.DestroyActor"></a>
Command adaptation of __<font color="#7fb800">destroy()</font>__ in [carla.Actor](#carla.Actor) that tells the simulator to destroy this actor. It has no effect if the actor was already destroyed. When executed with __<font color="#7fb800">apply_batch_sync()</font>__ in [carla.Client](#carla.Client) there will be a <b>command.Response</b> that will return a boolean stating whether the actor was successfully destroyed.  

### Instance Variables
- <a name="command.DestroyActor.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  

### Methods
- <a name="command.DestroyActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  

---

## command.Response<a name="command.Response"></a>
States the result of executing a command as either the ID of the actor to whom the command was applied to (when succeeded) or an error string (when failed).  actor ID, depending on whether or not the command succeeded. The method __<font color="#7fb800">apply_batch_sync()</font>__ in [carla.Client](#carla.Client) returns a list of these to summarize the execution of a batch.  

### Instance Variables
- <a name="command.Response.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor to whom the command was applied to. States that the command was successful.  
- <a name="command.Response.error"></a>**<font color="#f8805a">error</font>** (_str_)  
A string stating the command has failed.  

### Methods
- <a name="command.Response.has_error"></a>**<font color="#7fb800">has_error</font>**(<font color="#00a6ed">**self**</font>)  
Returns <b>True</b> if the command execution fails, and <b>False</b> if it was successful.  
    - **Return:** _bool_  

---

## command.SetAutopilot<a name="command.SetAutopilot"></a>
Command adaptation of __<font color="#7fb800">set_autopilot()</font>__ in [carla.Vehicle](#carla.Vehicle). Turns on/off the vehicle's autopilot mode.  

### Instance Variables
- <a name="command.SetAutopilot.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor that is affected by the command.  
- <a name="command.SetAutopilot.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  
If autopilot should be activated or not.  
- <a name="command.SetAutopilot.port"></a>**<font color="#f8805a">port</font>** (_uint16_)  
Port of the Traffic Manager where the vehicle is to be registered or unlisted.  

### Methods
- <a name="command.SetAutopilot.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>, <font color="#00a6ed">**port**=8000</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `enabled` (_bool_)  
        - `port` (_uint16_) - The Traffic Manager port where the vehicle is to be registered or unlisted. If __None__ is passed, it will consider a TM at default port `8000`.  

---

## command.SetEnableGravity<a name="command.SetEnableGravity"></a>
Command adaptation of __<font color="#7fb800">set_enable_gravity()</font>__ in [carla.Actor](#carla.Actor). Enables or disables gravity on an actor.  

### Instance Variables
- <a name="command.SetEnableGravity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_[carla.Actor](#carla.Actor) or int_)  
Actor that is affected by the command.  
- <a name="command.SetEnableGravity.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  

### Methods
- <a name="command.SetEnableGravity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or Actor ID to which the command will be applied to.  
        - `enabled` (_bool_)  

---

## command.SetSimulatePhysics<a name="command.SetSimulatePhysics"></a>
Command adaptation of __<font color="#7fb800">set_simulate_physics()</font>__ in [carla.Actor](#carla.Actor). Determines whether an actor will be affected by physics or not.  

### Instance Variables
- <a name="command.SetSimulatePhysics.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.SetSimulatePhysics.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  
If physics should be activated or not.  

### Methods
- <a name="command.SetSimulatePhysics.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `enabled` (_bool_)  

---

## command.SetVehicleLightState<a name="command.SetVehicleLightState"></a>
Command adaptation of __<font color="#7fb800">set_light_state()</font>__ in [carla.Vehicle](#carla.Vehicle). Sets the light state of a vehicle.  

### Instance Variables
- <a name="command.SetVehicleLightState.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor that is affected by the command.  
- <a name="command.SetVehicleLightState.light_state"></a>**<font color="#f8805a">light_state</font>** (_[carla.VehicleLightState](#carla.VehicleLightState)_)  
Defines the light state of a vehicle.  

### Methods
- <a name="command.SetVehicleLightState.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**light_state**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or its ID to whom the command will be applied to.  
        - `light_state` (_[carla.VehicleLightState](#carla.VehicleLightState)_) - Recaps the state of the lights of a vehicle, these can be used as a flags.  

---

## command.ShowDebugTelemetry<a name="command.ShowDebugTelemetry"></a>
Command adaptation of __<font color="#7fb800">show_debug_telemetry()</font>__ in [carla.Actor](#carla.Actor). Displays vehicle control telemetry data.  

### Instance Variables
- <a name="command.ShowDebugTelemetry.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_[carla.Actor](#carla.Actor) or int_)  
Actor that is affected by the command.  
- <a name="command.ShowDebugTelemetry.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  

### Methods
- <a name="command.ShowDebugTelemetry.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_) - Actor or Actor ID to which the command will be applied to.  
        - `enabled` (_bool_)  

---

## command.SpawnActor<a name="command.SpawnActor"></a>
Command adaptation of __<font color="#7fb800">spawn_actor()</font>__ in [carla.World](#carla.World). Spawns an actor into the world based on the blueprint provided and the transform. If a parent is provided, the actor is attached to it.  

### Instance Variables
- <a name="command.SpawnActor.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transform to be applied.  
- <a name="command.SpawnActor.parent_id"></a>**<font color="#f8805a">parent_id</font>** (_int_)  
Identificator of the parent actor.  

### Methods
- <a name="command.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="command.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>)  
    - **Parameters:**
        - `blueprint` (_[carla.ActorBlueprint](#carla.ActorBlueprint)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
- <a name="command.SpawnActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**parent**</font>)  
    - **Parameters:**
        - `blueprint` (_[carla.ActorBlueprint](#carla.ActorBlueprint)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
        - `parent` (_[carla.Actor](#carla.Actor) or int_)  
- <a name="command.SpawnActor.then"></a>**<font color="#7fb800">then</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**command**</font>)  
Links another command to be executed right after. It allows to ease very common flows such as spawning a set of vehicles by command and then using this method to set them to autopilot automatically.  
    - **Parameters:**
        - `command` (_any carla Command_) - a Carla command.  

---
[comment]: <> (=========================)
[comment]: <> (PYTHON API SCRIPT SNIPETS)
[comment]: <> (=========================)
<div id="snipets-container" class="Container" onmouseover='this.style["overflowX"]="scroll";' onmouseout='this.style["overflowX"]="visible";'></div>
<script>
function CopyToClipboard(containerid) {
if (document.selection) {
var range = document.body.createTextRange();
range.moveToElementText(document.getElementById(containerid));
range.select().createTextRange();
document.execCommand("copy");
} 
else if (window.getSelection) {
var range = document.createRange();
range.selectNode(document.getElementById(containerid));
window.getSelection().addRange(range);
document.execCommand("copy");
}
}
</script>
<script>
function CloseSnipet() {
document.getElementById("snipets-container").innerHTML = null;
}
</script>
  
<div id ="carla.World.enable_environment_objects-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.enable_environment_objects
</p>
<div id="carla.World.enable_environment_objects-code" class="SnipetContent">

```py
  
# This recipe turn visibility off and on for two specifc buildings on the map

# Get the buildings in the world
world = client.get_world()
env_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)

# Access individual building IDs and save in a set
building_01 = env_objs[0]
building_02 = env_objs[1]
objects_to_toggle = {building_01.id, building_02.id}

# Toggle buildings off
world.enable_environment_objects(objects_to_toggle, False)
# Toggle buildings on
world.enable_environment_objects(objects_to_toggle, True)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.enable_environment_objects-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.DebugHelper.draw_string-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.DebugHelper.draw_string
</p>
<div id="carla.DebugHelper.draw_string-code" class="SnipetContent">

```py
  

# This recipe is a modification of lane_explorer.py example.
# It draws the path of an actor through the world, printing information at each waypoint.

# ...
current_w = map.get_waypoint(vehicle.get_location())
while True:

    next_w = map.get_waypoint(vehicle.get_location(), lane_type=carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk )
    # Check if the vehicle is moving
    if next_w.id != current_w.id:
        vector = vehicle.get_velocity()
        # Check if the vehicle is on a sidewalk
        if current_w.lane_type == carla.LaneType.Sidewalk:
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else red, 60)
        else:
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else green, 60)
        debug.draw_string(current_w.transform.location, str('%15.0f km/h' % (3.6 * math.sqrt(vector.x**2 + vector.y**2 + vector.z**2))), False, orange, 60)
        draw_transform(debug, current_w.transform, white, 60)

    # Update the current waypoint and sleep for some time
    current_w = next_w
    time.sleep(args.tick_time)
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.DebugHelper.draw_string-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.World.unload_map_layer-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.unload_map_layer
</p>
<div id="carla.World.unload_map_layer-code" class="SnipetContent">

```py
  
# This recipe toggles off several layers in our "_Opt" maps

# Load town one with minimum layout (roads, sidewalks, traffic lights and traffic signs)
# as well as buildings and parked vehicles
world = client.load_world('Town01_Opt', carla.MapLayer.Buildings | carla.MapLayer.ParkedVehicles) 

# Toggle all buildings off
world.unload_map_layer(carla.MapLayer.Buildings)

# Toggle all parked vehicles off
world.unload_map_layer(carla.MapLayer.ParkedVehicles)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.unload_map_layer-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Vehicle.set_wheel_steer_direction-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Vehicle.set_wheel_steer_direction
</p>
<div id="carla.Vehicle.set_wheel_steer_direction-code" class="SnipetContent">

```py
  
# Sets the appearance of the vehicles front wheels to 40°. Vehicle physics will not be affected.

vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FR_Wheel, 40.0)
vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FL_Wheel, 40.0)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Vehicle.set_wheel_steer_direction-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Client.__init__-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Client.__init__
</p>
<div id="carla.Client.__init__-code" class="SnipetContent">

```py
  

# This recipe shows in every script provided in PythonAPI/Examples 
# and it is used to parse the client creation arguments when running the script. 

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-s', '--speed',
        metavar='FACTOR',
        default=1.0,
        type=float,
        help='rate at which the weather changes (default: 1.0)')
    args = argparser.parse_args()

    speed_factor = args.speed
    update_freq = 0.1 / speed_factor

    client = carla.Client(args.host, args.port)

  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Client.__init__-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Map.get_waypoint-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Map.get_waypoint
</p>
<div id="carla.Map.get_waypoint-code" class="SnipetContent">

```py
  

# This recipe shows the current traffic rules affecting the vehicle. 
# Shows the current lane type and if a lane change can be done in the actual lane or the surrounding ones.

# ...
waypoint = world.get_map().get_waypoint(vehicle.get_location(),project_to_road=True, lane_type=(carla.LaneType.Driving | carla.LaneType.Shoulder | carla.LaneType.Sidewalk))
print("Current lane type: " + str(waypoint.lane_type))
# Check current lane change allowed
print("Current Lane change:  " + str(waypoint.lane_change))
# Left and Right lane markings
print("L lane marking type: " + str(waypoint.left_lane_marking.type))
print("L lane marking change: " + str(waypoint.left_lane_marking.lane_change))
print("R lane marking type: " + str(waypoint.right_lane_marking.type))
print("R lane marking change: " + str(waypoint.right_lane_marking.lane_change))
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Map.get_waypoint-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  

<img src="/img/snipets_images/carla.Map.get_waypoint.jpg">
  
</div>
  
<div id ="carla.World.spawn_actor-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.spawn_actor
</p>
<div id="carla.World.spawn_actor-code" class="SnipetContent">

```py
  

# This recipe attaches different camera / sensors to a vehicle with different attachments.

# ...
camera = world.spawn_actor(rgb_camera_bp, transform, attach_to=vehicle, attachment_type=Attachment.Rigid)
# Default attachment:  Attachment.Rigid
gnss_sensor = world.spawn_actor(sensor_gnss_bp, transform, attach_to=vehicle)
collision_sensor = world.spawn_actor(sensor_collision_bp, transform, attach_to=vehicle)
lane_invasion_sensor = world.spawn_actor(sensor_lane_invasion_bp, transform, attach_to=vehicle)
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.spawn_actor-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.WalkerAIController.stop-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.WalkerAIController.stop
</p>
<div id="carla.WalkerAIController.stop-code" class="SnipetContent">

```py
  

#To destroy the pedestrians, stop them from the navigation, and then destroy the objects (actor and controller).

# stop pedestrians (list is [controller, actor, controller, actor ...])
for i in range(0, len(all_id), 2):
    all_actors[i].stop()

# destroy pedestrian (actor and controller)
client.apply_batch([carla.command.DestroyActor(x) for x in all_id])
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.WalkerAIController.stop-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.DebugHelper.draw_box-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.DebugHelper.draw_box
</p>
<div id="carla.DebugHelper.draw_box-code" class="SnipetContent">

```py
  

# This recipe shows how to draw traffic light actor bounding boxes from a world snapshot.

# ....
debug = world.debug
world_snapshot = world.get_snapshot()

for actor_snapshot in world_snapshot:
    actual_actor = world.get_actor(actor_snapshot.id)
    if actual_actor.type_id == 'traffic.traffic_light':
        debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
# ...

  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.DebugHelper.draw_box-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  

<img src="/img/snipets_images/carla.DebugHelper.draw_box.jpg">
  
</div>
  
<div id ="carla.World.get_spectator-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.get_spectator
</p>
<div id="carla.World.get_spectator-code" class="SnipetContent">

```py
  

# This recipe spawns an actor and the spectator camera at the actor's location.

# ...
world = client.get_world()
spectator = world.get_spectator()

vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
transform = random.choice(world.get_map().get_spawn_points())
vehicle = world.try_spawn_actor(vehicle_bp, transform)

# Wait for world to get the vehicle actor
world.tick()

world_snapshot = world.wait_for_tick()
actor_snapshot = world_snapshot.find(vehicle.id)

# Set spectator at given transform (vehicle transform)
spectator.set_transform(actor_snapshot.get_transform())
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.get_spectator-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Sensor.listen-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Sensor.listen
</p>
<div id="carla.Sensor.listen-code" class="SnipetContent">

```py
  

# This recipe applies a color conversion to the image taken by a camera sensor,
# so it is converted to a semantic segmentation image.

# ...
camera_bp = world.get_blueprint_library().filter('sensor.camera.semantic_segmentation')
# ...
cc = carla.ColorConverter.CityScapesPalette
camera.listen(lambda image: image.save_to_disk('output/%06d.png' % image.frame, cc))
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Sensor.listen-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.TrafficLight.set_state-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.TrafficLight.set_state
</p>
<div id="carla.TrafficLight.set_state-code" class="SnipetContent">

```py
  

# This recipe changes from red to green the traffic light that affects the vehicle. 
# This is done by detecting if the vehicle actor is at a traffic light.

# ...
world = client.get_world()
spectator = world.get_spectator()

vehicle_bp = random.choice(world.get_blueprint_library().filter('vehicle.bmw.*'))
transform = random.choice(world.get_map().get_spawn_points())
vehicle = world.try_spawn_actor(vehicle_bp, transform)

# Wait for world to get the vehicle actor
world.tick()

world_snapshot = world.wait_for_tick()
actor_snapshot = world_snapshot.find(vehicle.id)

# Set spectator at given transform (vehicle transform)
spectator.set_transform(actor_snapshot.get_transform())
# ...# ...
if vehicle_actor.is_at_traffic_light():
    traffic_light = vehicle_actor.get_traffic_light()
    if traffic_light.get_state() == carla.TrafficLightState.Red:
       # world.hud.notification("Traffic light changed! Good to go!")
        traffic_light.set_state(carla.TrafficLightState.Green)
# ...

  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.TrafficLight.set_state-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  

<img src="/img/snipets_images/carla.TrafficLight.set_state.gif">
  
</div>
  
<div id ="carla.ActorBlueprint.set_attribute-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.ActorBlueprint.set_attribute
</p>
<div id="carla.ActorBlueprint.set_attribute-code" class="SnipetContent">

```py
  

# This recipe changes attributes of different type of blueprint actors.

# ...
walker_bp = world.get_blueprint_library().filter('walker.pedestrian.0002')
walker_bp.set_attribute('is_invincible', True)

# ...
# Changes attribute randomly by the recommended value
vehicle_bp = wolrd.get_blueprint_library().filter('vehicle.bmw.*')
color = random.choice(vehicle_bp.get_attribute('color').recommended_values)
vehicle_bp.set_attribute('color', color)

# ...

camera_bp = world.get_blueprint_library().filter('sensor.camera.rgb')
camera_bp.set_attribute('image_size_x', 600)
camera_bp.set_attribute('image_size_y', 600)
# ...
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.ActorBlueprint.set_attribute-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.Client.apply_batch_sync-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.Client.apply_batch_sync
</p>
<div id="carla.Client.apply_batch_sync-code" class="SnipetContent">

```py
  
# 0. Choose a blueprint fo the walkers
world = client.get_world()
blueprintsWalkers = world.get_blueprint_library().filter("walker.pedestrian.*")
walker_bp = random.choice(blueprintsWalkers)

# 1. Take all the random locations to spawn
spawn_points = []
for i in range(50):
    spawn_point = carla.Transform()
    spawn_point.location = world.get_random_location_from_navigation()
    if (spawn_point.location != None):
        spawn_points.append(spawn_point)

# 2. Build the batch of commands to spawn the pedestrians
batch = []
for spawn_point in spawn_points:
    walker_bp = random.choice(blueprintsWalkers)
    batch.append(carla.command.SpawnActor(walker_bp, spawn_point))

# 2.1 apply the batch
results = client.apply_batch_sync(batch, True)
for i in range(len(results)):
    if results[i].error:
        logging.error(results[i].error)
    else:
        walkers_list.append({"id": results[i].actor_id})

# 3. Spawn walker AI controllers for each walker
batch = []
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
for i in range(len(walkers_list)):
    batch.append(carla.command.SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))

# 3.1 apply the batch
results = client.apply_batch_sync(batch, True)
for i in range(len(results)):
    if results[i].error:
        logging.error(results[i].error)
    else:
        walkers_list[i]["con"] = results[i].actor_id

# 4. Put altogether the walker and controller ids
for i in range(len(walkers_list)):
    all_id.append(walkers_list[i]["con"])
    all_id.append(walkers_list[i]["id"])
all_actors = world.get_actors(all_id)

# wait for a tick to ensure client receives the last transform of the walkers we have just created
world.wait_for_tick()

# 5. initialize each controller and set target to walk to (list is [controller, actor, controller, actor ...])
for i in range(0, len(all_actors), 2):
    # start walker
    all_actors[i].start()
    # set walk to random point
    all_actors[i].go_to_location(world.get_random_location_from_navigation())
    # random max speed
    all_actors[i].set_max_speed(1 + random.random())    # max speed between 1 and 2 (default is 1.4 m/s)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.Client.apply_batch_sync-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  
<div id ="carla.World.load_map_layer-snipet" style="display: none;">
<p class="SnipetFont">
Snippet for carla.World.load_map_layer
</p>
<div id="carla.World.load_map_layer-code" class="SnipetContent">

```py
  
# This recipe toggles on several layers in our "_Opt" maps

# Load town one with only minimum layout (roads, sidewalks, traffic lights and traffic signs)
world = client.load_world('Town01_Opt', carla.MapLayer.None)

# Toggle all buildings on
world.load_map_layer(carla.MapLayer.Buildings)

# Toggle all foliage on
world.load_map_layer(carla.MapLayer.Foliage)

# Toggle all parked vehicles on
world.load_map_layer(carla.MapLayer.ParkedVehicles)
  

```
<button id="button1" class="CopyScript" onclick="CopyToClipboard('carla.World.load_map_layer-code')">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id="button1" class="CloseSnipet" onclick="CloseSnipet()">Close snippet</button><br><br>
  
</div>
  

</div>

<script>
function ButtonAction(container_name){
if(window_big){
snipet_name = container_name.replace('-snipet_button','-snipet');
document.getElementById("snipets-container").innerHTML = document.getElementById(snipet_name).innerHTML;
}
else{
document.getElementById("snipets-container").innerHTML = null;code_name = container_name.replace('-snipet_button','-code');
var range = document.createRange();
range.selectNode(document.getElementById(code_name));
alert(range);
}
}
function WindowResize(){
if(window.innerWidth > 1200){
window_big = true;
}
else{
window_big = false;
}
}
var window_big;
if(window.innerWidth > 1200){
window_big = true;
}
else{
window_big = false;
}
buttons = document.getElementsByClassName('SnipetButton')
for (let i = 0; i < buttons.length; i++) {
buttons[i].addEventListener("click",function(){ButtonAction(buttons[i].id);},true);
}
window.onresize = WindowResize;
</script>
