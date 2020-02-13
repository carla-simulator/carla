## carla.Actor<a name="carla.Actor"></a>
Base class for all actors.
Actor is anything that plays a role in the simulation and can be moved around, examples of actors are vehicles, pedestrians, and sensors.  

<h3>Instance Variables</h3>
- <a name="carla.Actor.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Unique id identifying this actor. Note ids are unique during a given episode.  
- <a name="carla.Actor.type_id"></a>**<font color="#f8805a">type_id</font>** (_str_)  
Id of the blueprint that created this actor, e.g. "vehicle.ford.mustang".  
- <a name="carla.Actor.parent"></a>**<font color="#f8805a">parent</font>** (_[carla.Actor](#carla.Actor)_)  
Parent actor of this instance, None if this instance is not attached to another actor.  
- <a name="carla.Actor.semantic_tags"></a>**<font color="#f8805a">semantic_tags</font>** (_list(int)_)  
List of semantic tags of all components of this actor, see semantic segmentation sensor for the list of available tags. E.g., a traffic light actor could contain "pole" and "traffic light" tags.  
- <a name="carla.Actor.is_alive"></a>**<font color="#f8805a">is_alive</font>** (_bool_)  
Returns whether this object was destroyed using this actor handle.  
- <a name="carla.Actor.attributes"></a>**<font color="#f8805a">attributes</font>** (_dict_)  
Dictionary of attributes of the blueprint that created this actor.  

<h3>Methods</h3>
- <a name="carla.Actor.destroy"></a>**<font color="#7fb800">destroy</font>**(<font color="#00a6ed">**self**</font>)  
Tell the simulator to destroy this Actor, and return whether the actor was successfully destroyed. It has no effect if the Actor was already successfully destroyed.  
    - **Return:** _bool_  
    - **Warning:** <font color="#ED2F2F">_This function blocks until the destruction operation is completed by the simulator.
_</font>  
- <a name="carla.Actor.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Returns the world this actor belongs to.  
    - **Return:** _[carla.World](#carla.World)_  
- <a name="carla.Actor.get_location"></a>**<font color="#7fb800">get_location</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current location.  
    - **Return:** _[carla.Location](#carla.Location)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the location received in the last tick.
_</font>  
- <a name="carla.Actor.get_transform"></a>**<font color="#7fb800">get_transform</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current transform.  
    - **Return:** _[carla.Transform](#carla.Transform)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the transform received in the last tick.
_</font>  
- <a name="carla.Actor.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current 3D velocity.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the velocity received in the last tick.
_</font>  
- <a name="carla.Actor.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current 3D angular velocity.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the angular velocity received in the last tick.
_</font>  
- <a name="carla.Actor.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current 3D acceleration.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the acceleration received in the last tick.
_</font>  
- <a name="carla.Actor.set_location"></a>**<font color="#7fb800">set_location</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Teleport the actor to a given location.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
- <a name="carla.Actor.set_transform"></a>**<font color="#7fb800">set_transform</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**transform**</font>)  
Teleport the actor to a given transform.  
    - **Parameters:**
        - `transform` (_[carla.Transform](#carla.Transform)_)  
- <a name="carla.Actor.set_velocity"></a>**<font color="#7fb800">set_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**velocity**</font>)  
Set the actor's velocity.  
    - **Parameters:**
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.set_angular_velocity"></a>**<font color="#7fb800">set_angular_velocity</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**angular_velocity**</font>)  
Set the actor's angular velocity.  
    - **Parameters:**
        - `angular_velocity` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.add_impulse"></a>**<font color="#7fb800">add_impulse</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**impulse**</font>)  
Add impulse to the actor.  
    - **Parameters:**
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Actor.set_simulate_physics"></a>**<font color="#7fb800">set_simulate_physics</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>)  
Enable or disable physics simulation on this actor.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Actor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _str_  

---

## carla.ActorAttribute<a name="carla.ActorAttribute"></a>
Class that defines an attribute of a [carla.ActorBlueprint](#carla.ActorBlueprint).  

<h3>Instance Variables</h3>
- <a name="carla.ActorAttribute.id"></a>**<font color="#f8805a">id</font>** (_str_)  
The attribute's identifier.  
- <a name="carla.ActorAttribute.type"></a>**<font color="#f8805a">type</font>** (_[carla.ActorAttributeType](#carla.ActorAttributeType)_)  
The attribute parameter type.  
- <a name="carla.ActorAttribute.recommended_values"></a>**<font color="#f8805a">recommended_values</font>** (_list(str)_)  
List of recommended values that the attribute may have.  
- <a name="carla.ActorAttribute.is_modifiable"></a>**<font color="#f8805a">is_modifiable</font>** (_bool_)  
True if the attribute is modifiable.  

<h3>Methods</h3>
- <a name="carla.ActorAttribute.as_bool"></a>**<font color="#7fb800">as_bool</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_int"></a>**<font color="#7fb800">as_int</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_float"></a>**<font color="#7fb800">as_float</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_str"></a>**<font color="#7fb800">as_str</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.as_color"></a>**<font color="#7fb800">as_color</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_bool / int / float / str / [carla.Color](#carla.Color) / [carla.ActorAttribute](#carla.ActorAttribute)_)  
- <a name="carla.ActorAttribute.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_bool / int / float / str / [carla.Color](#carla.Color) / [carla.ActorAttribute](#carla.ActorAttribute)_)  
- <a name="carla.ActorAttribute.__nonzero__"></a>**<font color="#7fb800">\__nonzero__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__bool__"></a>**<font color="#7fb800">\__bool__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__int__"></a>**<font color="#7fb800">\__int__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__float__"></a>**<font color="#7fb800">\__float__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorAttribute.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorAttributeType<a name="carla.ActorAttributeType"></a>
Class that defines the type of attribute of a [carla.ActorAttribute](#carla.ActorAttribute).  

<h3>Instance Variables</h3>
- <a name="carla.ActorAttributeType.Bool"></a>**<font color="#f8805a">Bool</font>**  
- <a name="carla.ActorAttributeType.Int"></a>**<font color="#f8805a">Int</font>**  
- <a name="carla.ActorAttributeType.Float"></a>**<font color="#f8805a">Float</font>**  
- <a name="carla.ActorAttributeType.String"></a>**<font color="#f8805a">String</font>**  
- <a name="carla.ActorAttributeType.RGBColor"></a>**<font color="#f8805a">RGBColor</font>**  

---

## carla.ActorBlueprint<a name="carla.ActorBlueprint"></a>
Class that contains all the necessary information for spawning an Actor.  

<h3>Instance Variables</h3>
- <a name="carla.ActorBlueprint.id"></a>**<font color="#f8805a">id</font>** (_str_)  
Actor blueprint identifier, e.g. `walker.pedestrian.0001`.  
- <a name="carla.ActorBlueprint.tags"></a>**<font color="#f8805a">tags</font>** (_list(str)_)  
List of tags of an actor blueprint e.g. `['0001', 'pedestrian', 'walker']`.  

<h3>Methods</h3>
- <a name="carla.ActorBlueprint.has_tag"></a>**<font color="#7fb800">has_tag</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**tag**</font>)  
Returns `true` if an actor blueprint has the tag.  
    - **Parameters:**
        - `tag` (_str_) – e.g. 'walker'.  
    - **Return:** _bool_  
- <a name="carla.ActorBlueprint.match_tags"></a>**<font color="#7fb800">match_tags</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Test if any of the flags or id matches wildcard_pattern.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _bool_  
    - **Note:** <font color="#8E8E8E">_The wildcard_pattern follows Unix shell-style wildcards.
_</font>  
- <a name="carla.ActorBlueprint.has_attribute"></a>**<font color="#7fb800">has_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
Returns `true` if the blueprint contains the specified attribute.  
    - **Parameters:**
        - `id` (_str_) – e.g 'gender'.  
    - **Return:** _bool_  
- <a name="carla.ActorBlueprint.get_attribute"></a>**<font color="#7fb800">get_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
Returns the current actor attribute through its id.  
    - **Parameters:**
        - `id` (_str_)  
    - **Return:** _[carla.ActorAttribute](#carla.ActorAttribute)_  
- <a name="carla.ActorBlueprint.set_attribute"></a>**<font color="#7fb800">set_attribute</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>, <font color="#00a6ed">**value**</font>)  
Sets an existing attribute to the actor's blueprint.  
    - **Parameters:**
        - `id` (_str_)  
        - `value` (_str_)  
    - **Note:** <font color="#8E8E8E">_Attribute can only be set or changed if it is modifiable
_</font>  
- <a name="carla.ActorBlueprint.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorBlueprint.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorList<a name="carla.ActorList"></a>
Class that provides access to actors.  

<h3>Methods</h3>
- <a name="carla.ActorList.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Find an actor by ID.  
    - **Parameters:**
        - `actor_id` (_int_)  
- <a name="carla.ActorList.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Filters a list of Actors with type_id matching wildcard_pattern.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Note:** <font color="#8E8E8E">_The wildcard_pattern follows Unix shell-style wildcards (fnmatch). 
_</font>  
- <a name="carla.ActorList.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
- <a name="carla.ActorList.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.ActorList.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ActorSnapshot<a name="carla.ActorSnapshot"></a>
Class that provides access to the data of a [carla.Actor](#carla.Actor) in a [carla.WorldSnapshot](#carla.WorldSnapshot).  

<h3>Instance Variables</h3>
- <a name="carla.ActorSnapshot.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The ActorSnapshot's identifier.  

<h3>Methods</h3>
- <a name="carla.ActorSnapshot.get_transform"></a>**<font color="#7fb800">get_transform</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current transform.  
    - **Return:** _[carla.Transform](#carla.Transform)_  
- <a name="carla.ActorSnapshot.get_velocity"></a>**<font color="#7fb800">get_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current 3D velocity.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.ActorSnapshot.get_angular_velocity"></a>**<font color="#7fb800">get_angular_velocity</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current 3D angular velocity.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.ActorSnapshot.get_acceleration"></a>**<font color="#7fb800">get_acceleration</font>**(<font color="#00a6ed">**self**</font>)  
Returns the actor's current 3D acceleration.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.ActorSnapshot.__self__"></a>**<font color="#7fb800">\__self__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.AttachmentType<a name="carla.AttachmentType"></a>
Class that defines the attachment options. See [carla.World.spawn_actor](#carla.World.spawn_actor).<br> Check out this [`recipe`](../python_cookbook/#attach-sensors-recipe)!  

<h3>Instance Variables</h3>
- <a name="carla.AttachmentType.Rigid"></a>**<font color="#f8805a">Rigid</font>**  
Standard fixed attachment.  
- <a name="carla.AttachmentType.SpringArm"></a>**<font color="#f8805a">SpringArm</font>**  
Attachment that expands or retracts based on camera situation.  

---

## carla.BlueprintLibrary<a name="carla.BlueprintLibrary"></a>
Class that provides access to [blueprints](../bp_library/).  

<h3>Methods</h3>
- <a name="carla.BlueprintLibrary.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**id**</font>)  
Returns a [carla.ActorBlueprint](#carla.ActorBlueprint) through its id.  
    - **Parameters:**
        - `id` (_str_)  
    - **Return:** _[carla.ActorBlueprint](#carla.ActorBlueprint)_  
- <a name="carla.BlueprintLibrary.filter"></a>**<font color="#7fb800">filter</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**wildcard_pattern**</font>)  
Filters a list of ActorBlueprint with id or tags matching wildcard_pattern. The pattern is matched against each blueprint's id and tags.  
    - **Parameters:**
        - `wildcard_pattern` (_str_)  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
    - **Note:** <font color="#8E8E8E">_The wildcard_pattern follows Unix shell-style wildcards (fnmatch).
_</font>  
- <a name="carla.BlueprintLibrary.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
    - **Return:** _[carla.ActorBlueprint](#carla.ActorBlueprint)_  
- <a name="carla.BlueprintLibrary.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.BlueprintLibrary.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.BoundingBox<a name="carla.BoundingBox"></a>
Bounding box helper class.  

<h3>Instance Variables</h3>
- <a name="carla.BoundingBox.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)_)  
The center of the bounding box relative to its parent actor.  
- <a name="carla.BoundingBox.extent"></a>**<font color="#f8805a">extent</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
It contains the vector from the center of the bounding box to one of the vertex of the box.
So, if you want to know the _X bounding box size_, you can just do `extent.x * 2`.  

<h3>Methods</h3>
- <a name="carla.BoundingBox.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**extent**</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `extent` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.BoundingBox.contains"></a>**<font color="#7fb800">contains</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**world_point**</font>, <font color="#00a6ed">**transform**</font>)  
Returns **True** if a point passed in world space is inside this bounding box.  
    - **Parameters:**
        - `world_point` (_[carla.Location](#carla.Location)_) – The point in world space to be checked.  
        - `transform` (_[carla.Transform](#carla.Transform)_) – Contains location and rotation needed to convert this object's local space to world space.  
    - **Return:** _bool_  
- <a name="carla.BoundingBox.get_local_vertices"></a>**<font color="#7fb800">get_local_vertices</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list containing the locations of this object's vertices in local space.  
    - **Return:** _list([carla.Location](#carla.Location))_  
- <a name="carla.BoundingBox.get_world_vertices"></a>**<font color="#7fb800">get_world_vertices</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**transform**</font>)  
Returns a list containing the locations of this object's vertices in world space.  
    - **Parameters:**
        - `transform` (_[carla.Transform](#carla.Transform)_) – Contains location and rotation needed to convert this object's local space to world space.  
    - **Return:** _list([carla.Location](#carla.Location))_  
- <a name="carla.BoundingBox.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.BoundingBox](#carla.BoundingBox)_)  
- <a name="carla.BoundingBox.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.BoundingBox](#carla.BoundingBox)_)  
- <a name="carla.BoundingBox.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Client<a name="carla.Client"></a>
Client used to connect to a Carla server.  

<h3>Methods</h3>
- <a name="carla.Client.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**host**</font>, <font color="#00a6ed">**port**</font>, <font color="#00a6ed">**worker_threads**=0</font>)  
Client constructor.  
    - **Parameters:**
        - `host` (_str_) – IP address where a CARLA Simulator instance is running.  
        - `port` (_int_) – TCP port where the CARLA Simulator instance is running.  
        - `worker_threads` (_int_) – Number of working threads used for background updates. If 0, use all available concurrency.  
- <a name="carla.Client.set_timeout"></a>**<font color="#7fb800">set_timeout</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**</font>)  
Set the timeout in seconds allowed to block when doing networking calls.  
    - **Parameters:**
        - `seconds` (_float_) – New timeout value in seconds.  
- <a name="carla.Client.get_client_version"></a>**<font color="#7fb800">get_client_version</font>**(<font color="#00a6ed">**self**</font>)  
Get the client version as a string.  
    - **Return:** _str_  
- <a name="carla.Client.get_server_version"></a>**<font color="#7fb800">get_server_version</font>**(<font color="#00a6ed">**self**</font>)  
Get the server version as a string.  
    - **Return:** _str_  
- <a name="carla.Client.get_world"></a>**<font color="#7fb800">get_world</font>**(<font color="#00a6ed">**self**</font>)  
Get the world currently active in the simulation.  
    - **Return:** _[carla.World](#carla.World)_  
- <a name="carla.Client.get_available_maps"></a>**<font color="#7fb800">get_available_maps</font>**(<font color="#00a6ed">**self**</font>)  
Get a list of strings of the maps available on server. The result can be something like:  
  '/Game/Carla/Maps/Town01'  
  '/Game/Carla/Maps/Town02'  
  '/Game/Carla/Maps/Town03'  
  '/Game/Carla/Maps/Town04'  
  '/Game/Carla/Maps/Town05'  
  '/Game/Carla/Maps/Town06'  
  '/Game/Carla/Maps/Town07'.  
    - **Return:** _list(str)_  
- <a name="carla.Client.reload_world"></a>**<font color="#7fb800">reload_world</font>**(<font color="#00a6ed">**self**</font>)  
Reload the current world, note that a new world is created with default settings using the same map. All actors present in the world will be destroyed.  
    - **Raises:** RuntimeError  
- <a name="carla.Client.load_world"></a>**<font color="#7fb800">load_world</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**map_name**</font>)  
Load a new world with default settings using `map_name` map. All actors present in the current world will be destroyed.  
    - **Parameters:**
        - `map_name` (_str_) – Name of the map to load, accepts both full paths and map names, e.g. '/Game/Carla/Maps/Town01' or 'Town01'.  
- <a name="carla.Client.start_recorder"></a>**<font color="#7fb800">start_recorder</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>)  
If we use a simple name like 'recording.log' then it will be saved at server folder 'CarlaUE4/Saved/recording.log'. If we use some folder in the name, then it will be considered to be an absolute path, like '/home/carla/recording.log'.  
    - **Parameters:**
        - `filename` (_str_) – Name of the file to write the recorded data.  
- <a name="carla.Client.stop_recorder"></a>**<font color="#7fb800">stop_recorder</font>**(<font color="#00a6ed">**self**</font>)  
Stops the recording in progress.  
- <a name="carla.Client.show_recorder_file_info"></a>**<font color="#7fb800">show_recorder_file_info</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**show_all**</font>)  
Will show info about the recorded file (frames, times, events, state, positions...) We have the option to show all the details per frame, that includes all the traffic light states, position of all actors, and animations data.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `show_all` (_bool_) – Show all detailed info, or just a summary.  
- <a name="carla.Client.show_recorder_collisions"></a>**<font color="#7fb800">show_recorder_collisions</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**category1**</font>, <font color="#00a6ed">**category2**</font>)  
This will show which collisions were recorded in the file. We can use a filter for the collisions we want, using two categories. The categories can be:  
  'h' = Hero  
  'v' = Vehicle  
  'w' = Walker  
  't' = Traffic light  
  'o' = Other  
  'a' = Any  
So, if you want to see only collisions about a vehicle and a walker, we would use for category1 'v' and category2 'w'. Or if you want all the collisions (filter off) you can use 'a' as both categories.  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `category1` (_single char_) – Character specifying the category of the first actor.  
        - `category2` (_single char_) – Character specifying the category of the second actor.  
- <a name="carla.Client.show_recorder_actors_blocked"></a>**<font color="#7fb800">show_recorder_actors_blocked</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**filename**</font>, <font color="#00a6ed">**min_time**</font>, <font color="#00a6ed">**min_distance**</font>)  
Shows which actors seem blocked by some reason. The idea is to calculate which actors are not moving as much as 'min_distance' for a period of 'min_time'. By default min_time = 60 seconds (1 min) and min_distance = 100 centimeters (1 m).  
    - **Parameters:**
        - `filename` (_str_) – Name of the recorded file to load.  
        - `min_time` (_float_) – How many seconds has to be stoped an actor to be considered as blocked.  
        - `min_distance` (_float_) – How many centimeters needs to move the actor in order to be considered as moving, and not blocked.  
- <a name="carla.Client.replay_file"></a>**<font color="#7fb800">replay_file</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**start**</font>, <font color="#00a6ed">**duration**</font>, <font color="#00a6ed">**follow_id**</font>)  
Playback a file.  
    - **Parameters:**
        - `name` (_str_) – Name of the file.  
        - `start` (_float_) – Time in seconds where to start the playback. If it is negative, then it starts from the end.  
        - `duration` (_float_) – Id of the actor to follow. If this is 0 then camera is disabled.  
        - `follow_id` (_int_)  
- <a name="carla.Client.set_replayer_time_factor"></a>**<font color="#7fb800">set_replayer_time_factor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**time_factor**</font>)  
Apply a different playback speed to current playback. Can be used several times while a playback is in curse.  
    - **Parameters:**
        - `time_factor` (_float_) – A value of 1.0 means normal time factor. A value < 1.0 means slow motion (for example 0.5 is half speed) A value > 1.0 means fast motion (for example 2.0 is double speed).  
- <a name="carla.Client.apply_batch"></a>**<font color="#7fb800">apply_batch</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**commands**</font>)  
This function executes the whole list of commands on a single simulation step. For example, to set autopilot on some actors, we could use:   [sample_code](https://github.com/carla-simulator/carla/blob/10c5f6a482a21abfd00220c68c7f12b4110b7f63/PythonAPI/examples/spawn_npc.py#L126).   We don't have control about the response of each command. If we need that, we can use `apply_batch_sync()`.  
    - **Parameters:**
        - `commands` (_list_) – A list of commands to execute in batch. Each command has a different number of parameters. Currently, we can use these [commands](#command.ApplyAngularVelocity):  
  SpawnActor  
  DestroyActor  
  ApplyVehicleControl  
  ApplyWalkerControl  
  ApplyTransform  
  ApplyVelocity  
  AplyAngularVelocity  
  ApplyImpulse  
  SetSimulatePhysics  
  SetAutopilot.  
- <a name="carla.Client.apply_batch_sync"></a>**<font color="#7fb800">apply_batch_sync</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**commands**</font>, <font color="#00a6ed">**due_tick_cue**</font>)  
This function executes the whole list of commands on a single simulation step, blocks until the commands are executed, and returns a list of [`command.Response`](#command.Response) that can be used to determine whether a single command succeeded or not. [sample_code](https://github.com/carla-simulator/carla/blob/10c5f6a482a21abfd00220c68c7f12b4110b7f63/PythonAPI/examples/spawn_npc.py#L112-L116).  
    - **Parameters:**
        - `commands` (_list_) – A list of commands to execute in batch. For a list of commands available see function above apply_batch().  
        - `due_tick_cue` (_bool_) – A boolean parameter to specify whether or not to perform a [carla.World.tick](#carla.World.tick) after applying the batch in _synchronous mode_.  
    - **Return:** _list_  

---

## carla.CollisionEvent<a name="carla.CollisionEvent"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Class that defines a registered collision.  

<h3>Instance Variables</h3>
- <a name="carla.CollisionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get "self" actor. Actor that measured the collision.  
- <a name="carla.CollisionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get the actor to which we collided.  
- <a name="carla.CollisionEvent.normal_impulse"></a>**<font color="#f8805a">normal_impulse</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Normal impulse result of the collision.  

---

## carla.Color<a name="carla.Color"></a>
Class that defines a 32-bit BGRA color.  

<h3>Instance Variables</h3>
- <a name="carla.Color.r"></a>**<font color="#f8805a">r</font>** (_int_)  
Red color (0-255).  
- <a name="carla.Color.g"></a>**<font color="#f8805a">g</font>** (_int_)  
Green color (0-255).  
- <a name="carla.Color.b"></a>**<font color="#f8805a">b</font>** (_int_)  
Blue color (0-255).  
- <a name="carla.Color.a"></a>**<font color="#f8805a">a</font>** (_int_)  
Alpha channel (0-255).  

<h3>Methods</h3>
- <a name="carla.Color.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**r**=0</font>, <font color="#00a6ed">**g**=0</font>, <font color="#00a6ed">**b**=0</font>, <font color="#00a6ed">**a**=255</font>)  
Client constructor.  
    - **Parameters:**
        - `r` (_int_)  
        - `g` (_int_)  
        - `b` (_int_)  
        - `a` (_int_)  
- <a name="carla.Color.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Color](#carla.Color)_)  
- <a name="carla.Color.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Color](#carla.Color)_)  
- <a name="carla.Color.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ColorConverter<a name="carla.ColorConverter"></a>
Class that defines the color converter options. Check out this [`recipe`](../python_cookbook/#converted-image-recipe)!  

<h3>Instance Variables</h3>
- <a name="carla.ColorConverter.Raw"></a>**<font color="#f8805a">Raw</font>**  
- <a name="carla.ColorConverter.Depth"></a>**<font color="#f8805a">Depth</font>**  
- <a name="carla.ColorConverter.LogarithmicDepth"></a>**<font color="#f8805a">LogarithmicDepth</font>**  
- <a name="carla.ColorConverter.CityScapesPalette"></a>**<font color="#f8805a">CityScapesPalette</font>**  

---

## carla.DebugHelper<a name="carla.DebugHelper"></a>
Class that provides drawing debug shapes. Check out this [`example`](https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/lane_explorer.py).  

<h3>Methods</h3>
- <a name="carla.DebugHelper.draw_point"></a>**<font color="#7fb800">draw_point</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**size**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
Draws a point in the given location.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `size` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time = 0` instead_.  
- <a name="carla.DebugHelper.draw_line"></a>**<font color="#7fb800">draw_line</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**begin**</font>, <font color="#00a6ed">**end**</font>, <font color="#00a6ed">**thickness**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
Draws a line between two given locations.  
    - **Parameters:**
        - `begin` (_[carla.Location](#carla.Location)_)  
        - `end` (_[carla.Location](#carla.Location)_)  
        - `thickness` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time = 0` instead_.  
- <a name="carla.DebugHelper.draw_arrow"></a>**<font color="#7fb800">draw_arrow</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**begin**</font>, <font color="#00a6ed">**end**</font>, <font color="#00a6ed">**thickness**=0.1f</font>, <font color="#00a6ed">**arrow_size**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
Draws an arrow between two given locations.  
    - **Parameters:**
        - `begin` (_[carla.Location](#carla.Location)_)  
        - `end` (_[carla.Location](#carla.Location)_)  
        - `thickness` (_float_)  
        - `arrow_size` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time = 0` instead_.  
- <a name="carla.DebugHelper.draw_box"></a>**<font color="#7fb800">draw_box</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**box**</font>, <font color="#00a6ed">**rotation**</font>, <font color="#00a6ed">**thickness**=0.1f</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
Draws the [carla.BoundingBox](#carla.BoundingBox) of a given bounding_box.<br> Check out this [`recipe`](../python_cookbook/#debug-bounding-box-recipe)!  
    - **Parameters:**
        - `box` (_[carla.BoundingBox](#carla.BoundingBox)_)  
        - `rotation` (_[carla.Rotation](#carla.Rotation)_)  
        - `thickness` (_float_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, use `life_time = 0` instead_.  
- <a name="carla.DebugHelper.draw_string"></a>**<font color="#7fb800">draw_string</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**text**</font>, <font color="#00a6ed">**draw_shadow**=False</font>, <font color="#00a6ed">**color**=(255,0,0)</font>, <font color="#00a6ed">**life_time**=-1.0f</font>, <font color="#00a6ed">**persistent_lines**=True</font>)  
Draws a string in a given location.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `text` (_str_)  
        - `draw_shadow` (_bool_)  
        - `color` (_[carla.Color](#carla.Color)_)  
        - `life_time` (_float_)  
        - `persistent_lines` (_bool_) – _Deprecated, set a high `life_time` instead_.  
    - **Note:** <font color="#8E8E8E">_Strings can only be seen on the server-side.
_</font>  

---

## carla.GearPhysicsControl<a name="carla.GearPhysicsControl"></a>
Class that provides access to vehicle transmission details.  

<h3>Instance Variables</h3>
- <a name="carla.GearPhysicsControl.ratio"></a>**<font color="#f8805a">ratio</font>** (_float_)  
The transmission ratio of the gear.  
- <a name="carla.GearPhysicsControl.down_ratio"></a>**<font color="#f8805a">down_ratio</font>** (_float_)  
The level of RPM (in relation to MaxRPM) where the gear autobox initiates shifting down.  
- <a name="carla.GearPhysicsControl.up_ratio"></a>**<font color="#f8805a">up_ratio</font>** (_float_)  
The level of RPM (in relation to MaxRPM) where the gear autobox initiates shifting up.  

<h3>Methods</h3>
- <a name="carla.GearPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**ratio**=1.0</font>, <font color="#00a6ed">**down_ratio**=0.5</font>, <font color="#00a6ed">**up_ratio**=0.65</font>)  
    - **Parameters:**
        - `ratio` (_float_)  
        - `down_ratio` (_float_)  
        - `up_ratio` (_float_)  
- <a name="carla.GearPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.GearPhysicsControl](#carla.GearPhysicsControl)_)  
- <a name="carla.GearPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.GearPhysicsControl](#carla.GearPhysicsControl)_)  
- <a name="carla.GearPhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GeoLocation<a name="carla.GeoLocation"></a>
Class that contains geolocation simulated data.  

<h3>Instance Variables</h3>
- <a name="carla.GeoLocation.latitude"></a>**<font color="#f8805a">latitude</font>** (_float_)  
North/South value of a point on the map.  
- <a name="carla.GeoLocation.longitude"></a>**<font color="#f8805a">longitude</font>** (_float_)  
West/East value of a point on the map.  
- <a name="carla.GeoLocation.altitude"></a>**<font color="#f8805a">altitude</font>** (_float_)  
Height regarding ground level.  

<h3>Methods</h3>
- <a name="carla.GeoLocation.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**latitude**=0.0</font>, <font color="#00a6ed">**longitude**=0.0</font>, <font color="#00a6ed">**altitude**=0.0</font>)  
    - **Parameters:**
        - `latitude` (_float_)  
        - `longitude` (_float_)  
        - `altitude` (_float_)  
- <a name="carla.GeoLocation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.GeoLocation](#carla.GeoLocation)_)  
- <a name="carla.GeoLocation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.GeoLocation](#carla.GeoLocation)_)  
- <a name="carla.GeoLocation.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.GnssMeasurement<a name="carla.GnssMeasurement"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Gnss sensor data.  

<h3>Instance Variables</h3>
- <a name="carla.GnssMeasurement.latitude"></a>**<font color="#f8805a">latitude</font>** (_float_)  
North/South value of a point on the map.  
- <a name="carla.GnssMeasurement.longitude"></a>**<font color="#f8805a">longitude</font>** (_float_)  
West/East value of a point on the map.  
- <a name="carla.GnssMeasurement.altitude"></a>**<font color="#f8805a">altitude</font>** (_float_)  
Height regarding ground level.  

<h3>Methods</h3>
- <a name="carla.GnssMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.IMUMeasurement<a name="carla.IMUMeasurement"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>IMU sensor data regarding the sensor World's transformation.  

<h3>Instance Variables</h3>
- <a name="carla.IMUMeasurement.accelerometer"></a>**<font color="#f8805a">accelerometer</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Measures linear acceleration in `m/s^2`.  
- <a name="carla.IMUMeasurement.gyroscope"></a>**<font color="#f8805a">gyroscope</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Measures angular velocity in `rad/sec`.  
- <a name="carla.IMUMeasurement.compass"></a>**<font color="#f8805a">compass</font>** (_float_)  
Orientation with respect to the North (`(0.0, -1.0, 0.0)` in Unreal) in radians.  

<h3>Methods</h3>
- <a name="carla.IMUMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Image<a name="carla.Image"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Class that defines an image of 32-bit BGRA colors.  

<h3>Instance Variables</h3>
- <a name="carla.Image.width"></a>**<font color="#f8805a">width</font>** (_int_)  
Image width in pixels.  
- <a name="carla.Image.height"></a>**<font color="#f8805a">height</font>** (_int_)  
Image height in pixels.  
- <a name="carla.Image.fov"></a>**<font color="#f8805a">fov</font>** (_float_)  
Horizontal field of view of the image in degrees.  
- <a name="carla.Image.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  

<h3>Methods</h3>
- <a name="carla.Image.convert"></a>**<font color="#7fb800">convert</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**color_converter**</font>)  
Convert the image with the applied conversion.  
    - **Parameters:**
        - `color_converter` (_[carla.ColorConverter](#carla.ColorConverter)_)  
- <a name="carla.Image.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>, <font color="#00a6ed">**color_converter**=Raw</font>)  
Save the image to disk.  
    - **Parameters:**
        - `path` (_str_) – Path where it will be saved.  
        - `color_converter` (_[carla.ColorConverter](#carla.ColorConverter)_)  
- <a name="carla.Image.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.Image.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
- <a name="carla.Image.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>, <font color="#00a6ed">**color**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
        - `color` (_[carla.Color](#carla.Color)_)  
- <a name="carla.Image.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Junction<a name="carla.Junction"></a>
Class that embodies the intersections on the road described in the OpenDRIVE file according to OpenDRIVE 1.4 standards.  

<h3>Instance Variables</h3>
- <a name="carla.Junction.id"></a>**<font color="#f8805a">id</font>** (_int_)  
Identificator found in the OpenDRIVE file.  
- <a name="carla.Junction.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
Bounding box encapsulating the junction lanes.  

<h3>Methods</h3>
- <a name="carla.Junction.get_waypoints"></a>**<font color="#7fb800">get_waypoints</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**lane_type**</font>)  
Returns a list of pairs of waypoints. Every tuple on the list contains first an initial and then a final waypoint within the intersection boundaries that describe the beginning and the end of said lane along the junction. Lanes follow their OpenDRIVE definitions so there may be many different tuples with the same starting waypoint due to possible deviations, as this are considered different lanes.  
    - **Parameters:**
        - `lane_type` (_[carla.LaneType](#carla.LaneType)_) – Type of lanes to get the waypoints.  
    - **Return:** _list(tuple([carla.Waypoint](#carla.Waypoint)))_  

---

## carla.LaneChange<a name="carla.LaneChange"></a>
Class that defines the permission to turn either left, right, both or none (meaning only going straight is allowed). This information is stored for every [carla.Waypoint](#carla.Waypoint) according to the OpenDRIVE file. In this [recipe](../python_cookbook/#lanes-recipe) the user creates a waypoint for a current vehicle position and learns which turns are permitted.  

<h3>Instance Variables</h3>
- <a name="carla.LaneChange.NONE"></a>**<font color="#f8805a">NONE</font>**  
Traffic rules do not allow turning right or left, only going straight.  
- <a name="carla.LaneChange.Both"></a>**<font color="#f8805a">Both</font>**  
Traffic rules allow turning either right or left.  
- <a name="carla.LaneChange.Left"></a>**<font color="#f8805a">Left</font>**  
Traffic rules allow turning left.  
- <a name="carla.LaneChange.Right"></a>**<font color="#f8805a">Right</font>**  
Traffic rules allow turning right.  

---

## carla.LaneInvasionEvent<a name="carla.LaneInvasionEvent"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Lane invasion sensor data.  

<h3>Instance Variables</h3>
- <a name="carla.LaneInvasionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get "self" actor. Actor that invaded another lane.  
- <a name="carla.LaneInvasionEvent.crossed_lane_markings"></a>**<font color="#f8805a">crossed_lane_markings</font>** (_list([carla.LaneMarking](#carla.LaneMarking))_)  
List of lane markings that have been crossed.  

<h3>Methods</h3>
- <a name="carla.LaneInvasionEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.LaneMarking<a name="carla.LaneMarking"></a>
Class that gathers all the information regarding a lane marking according to [OpenDRIVE 1.4 standard](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf) standard.  

<h3>Instance Variables</h3>
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

<h3>Instance Variables</h3>
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
Class that defines the lane marking types accepted by OpenDRIVE 1.4. Take a look at this [recipe](../python_cookbook/#lanes-recipe) where the user creates a [carla.Waypoint](#carla.Waypoint) for a vehicle location and retrieves from it the information about adjacent lane markings.  

<h3>Instance Variables</h3>
- <a name="carla.LaneMarkingType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneMarkingType.BottsDots"></a>**<font color="#f8805a">BottsDots</font>**  
- <a name="carla.LaneMarkingType.Broken"></a>**<font color="#f8805a">Broken</font>**  
- <a name="carla.LaneMarkingType.BrokenBroken"></a>**<font color="#f8805a">BrokenBroken</font>**  
From inside to outside except for center lane which is from left to right.  
- <a name="carla.LaneMarkingType.BrokenSolid"></a>**<font color="#f8805a">BrokenSolid</font>**  
From inside to outside except for center lane which is from left to right.  
- <a name="carla.LaneMarkingType.Curb"></a>**<font color="#f8805a">Curb</font>**  
- <a name="carla.LaneMarkingType.Grass"></a>**<font color="#f8805a">Grass</font>**  
- <a name="carla.LaneMarkingType.Solid"></a>**<font color="#f8805a">Solid</font>**  
- <a name="carla.LaneMarkingType.SolidBroken"></a>**<font color="#f8805a">SolidBroken</font>**  
From inside to outside except for center lane which is from left to right.  
- <a name="carla.LaneMarkingType.SolidSolid"></a>**<font color="#f8805a">SolidSolid</font>**  
For double solid line.  
- <a name="carla.LaneMarkingType.Other"></a>**<font color="#f8805a">Other</font>**  

---

## carla.LaneType<a name="carla.LaneType"></a>
Class that defines the possible lane types accepted by OpenDRIVE 1.4. This standards define the road information. For instance in this [recipe](../python_cookbook/#lanes-recipe) the user creates a [carla.Waypoint](#carla.Waypoint) for the current location of a vehicle and uses it to get the current and adjacent lane types.  

<h3>Instance Variables</h3>
- <a name="carla.LaneType.NONE"></a>**<font color="#f8805a">NONE</font>**  
- <a name="carla.LaneType.Any"></a>**<font color="#f8805a">Any</font>**  
Every type except for NONE.  
- <a name="carla.LaneType.Bidirectional"></a>**<font color="#f8805a">Bidirectional</font>**  
- <a name="carla.LaneType.Biking"></a>**<font color="#f8805a">Biking</font>**  
- <a name="carla.LaneType.Border"></a>**<font color="#f8805a">Border</font>**  
- <a name="carla.LaneType.Driving"></a>**<font color="#f8805a">Driving</font>**  
- <a name="carla.LaneType.Entry"></a>**<font color="#f8805a">Entry</font>**  
- <a name="carla.LaneType.Exit"></a>**<font color="#f8805a">Exit</font>**  
- <a name="carla.LaneType.Median"></a>**<font color="#f8805a">Median</font>**  
- <a name="carla.LaneType.OffRamp"></a>**<font color="#f8805a">OffRamp</font>**  
- <a name="carla.LaneType.OnRamp"></a>**<font color="#f8805a">OnRamp</font>**  
- <a name="carla.LaneType.Parking"></a>**<font color="#f8805a">Parking</font>**  
- <a name="carla.LaneType.Rail"></a>**<font color="#f8805a">Rail</font>**  
- <a name="carla.LaneType.Restricted"></a>**<font color="#f8805a">Restricted</font>**  
- <a name="carla.LaneType.RoadWorks"></a>**<font color="#f8805a">RoadWorks</font>**  
- <a name="carla.LaneType.Shoulder"></a>**<font color="#f8805a">Shoulder</font>**  
- <a name="carla.LaneType.Sidewalk"></a>**<font color="#f8805a">Sidewalk</font>**  
- <a name="carla.LaneType.Special1"></a>**<font color="#f8805a">Special1</font>**  
- <a name="carla.LaneType.Special2"></a>**<font color="#f8805a">Special2</font>**  
- <a name="carla.LaneType.Special3"></a>**<font color="#f8805a">Special3</font>**  
- <a name="carla.LaneType.Stop"></a>**<font color="#f8805a">Stop</font>**  
- <a name="carla.LaneType.Tram"></a>**<font color="#f8805a">Tram</font>**  

---

## carla.LidarMeasurement<a name="carla.LidarMeasurement"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Lidar sensor measurement data.  

<h3>Instance Variables</h3>
- <a name="carla.LidarMeasurement.horizontal_angle"></a>**<font color="#f8805a">horizontal_angle</font>** (_float_)  
Horizontal angle that the Lidar has rotated at the time of the measurement (in radians).  
- <a name="carla.LidarMeasurement.channels"></a>**<font color="#f8805a">channels</font>** (_int_)  
Number of lasers.  
- <a name="carla.LidarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
List of 3D points.  

<h3>Methods</h3>
- <a name="carla.LidarMeasurement.get_point_count"></a>**<font color="#7fb800">get_point_count</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**channel**</font>)  
Retrieve the number of points that are generated by this channel.  
    - **Parameters:**
        - `channel` (_int_)  
    - **Note:** <font color="#8E8E8E">_Points are sorted by channel, so this method allows to identify the channel that generated each point.
_</font>  
- <a name="carla.LidarMeasurement.save_to_disk"></a>**<font color="#7fb800">save_to_disk</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Save point cloud to disk.  
    - **Parameters:**
        - `path` (_str_)  
- <a name="carla.LidarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.LidarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
- <a name="carla.LidarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>, <font color="#00a6ed">**location**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
        - `location` (_[carla.Location](#carla.Location)_)  
- <a name="carla.LidarMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Location<a name="carla.Location"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.Vector3D](#carla.Vector3D)_</b></small></div></p><p>Represents a location in the world (in meters).  

<h3>Instance Variables</h3>
- <a name="carla.Location.x"></a>**<font color="#f8805a">x</font>** (_float_)  
- <a name="carla.Location.y"></a>**<font color="#f8805a">y</font>** (_float_)  
- <a name="carla.Location.z"></a>**<font color="#f8805a">z</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Location.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>, <font color="#00a6ed">**z**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
        - `z` (_float_)  
- <a name="carla.Location.distance"></a>**<font color="#7fb800">distance</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Computes the Euclidean distance in meters from this location to another one.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) – The Location from where to compute the distance.  
    - **Return:** _float_  
- <a name="carla.Location.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Location](#carla.Location)_)  
- <a name="carla.Location.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Location](#carla.Location)_)  
- <a name="carla.Location.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Map<a name="carla.Map"></a>
Class containing the road information and waypoint managing. Data is retrieved from an OpenDRIVE file that describes the road. A query system is defined which works hand in hand with [carla.Waypoint](#carla.Waypoint) to translate geometrical information from the .xodr to natural world points. CARLA is currently working with [OpenDRIVE 1.4 standard](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf).  

<h3>Instance Variables</h3>
- <a name="carla.Map.name"></a>**<font color="#f8805a">name</font>** (_str_)  
The name of the map. It corresponds to the .umap from Unreal Engine that is loaded from a CARLA server, which then references to the .xodr road description.  

<h3>Methods</h3>
- <a name="carla.Map.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**name**</font>, <font color="#00a6ed">**xodr_content**</font>)  
Constructor for this class. Though a map is automatically generated when initializing the world, using this method in no-rendering mode facilitates working with an .xodr without any CARLA server running.  
    - **Parameters:**
        - `name` (_str_) – Name of the current map.  
        - `xodr_content` (_str_) – .xodr content in string format.  
    - **Return:** _list([carla.Transform](#carla.Transform))_  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#7fb800">generate_waypoints</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints with a certain distance between them for every lane and centered inside of it. Waypoints are not listed in any particular order. Remember that waypoints closer than 2cm within the same road, section and lane will have the same identificator.  
    - **Parameters:**
        - `distance` (_float_) – Approximate distance between waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Map.get_spawn_points"></a>**<font color="#7fb800">get_spawn_points</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of recommendations made by creators of the map to be used spawning points for vehicles. The list includes [carla.Tranform](#carla.Tranform) objects with certain location and orientation. Said locations are slightly on-air and vehicles fall for a bit before starting their way.  
    - **Return:** _list([carla.Transform](#carla.Transform))_  
- <a name="carla.Map.get_topology"></a>**<font color="#7fb800">get_topology</font>**(<font color="#00a6ed">**self**</font>)  
Returns a list of tuples describing a minimal graph of the topology of the OpenDRIVE file. The tuples contain pairs of waypoints located either at the point a road begins or ends. The first one is the origin and the second one represents another road end that can be reached. This graph can be loaded into [NetworkX](https://networkx.github.io/) to work with. Output could look like this: <b>[(w0, w1), (w0, w2), (w1, w3), (w2, w3), (w0, w4)]</b>.  
    - **Return:** _list(tuple([carla.Waypoint](#carla.Waypoint), [carla.Waypoint](#carla.Waypoint)))_  
- <a name="carla.Map.get_waypoint"></a>**<font color="#7fb800">get_waypoint</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**project_to_road**=True</font>, <font color="#00a6ed">**lane_type**=[carla.LaneType.Driving](#carla.LaneType.Driving)</font>)  
Returns a waypoint that can be located in an exact location or translated to the center of the nearest lane. Said lane type can be defined using flags such as `LaneType.Driving & LaneType.Shoulder`.
 The method will return <b>None</b> if the waypoint is not found, which may happen only when trying to retrieve a waypoint for an exact location. That eases checking if a point is inside a certain road, as otherwise, it will return the corresponding waypoint.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_) – Location used as reference for the [carla.Waypoint](#carla.Waypoint).  
        - `project_to_road` (_bool_) – If **True**, the waypoint will be at the center of the closest lane. This is the default setting. If **False**, the waypoint will be exactly in `location`. <b>None</b> means said location does not belong to a road.  
        - `lane_type` (_[carla.LaneType](#carla.LaneType)_) – Limits the search for nearest lane to one or various lane types that can be flagged.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Map.get_waypoint_xodr"></a>**<font color="#7fb800">get_waypoint_xodr</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**road_id**</font>, <font color="#00a6ed">**lane_id**</font>, <font color="#00a6ed">**s**</font>)  
Get a waypoint if all the parameters passed are correct, otherwise return None.  
    - **Parameters:**
        - `road_id` (_int_) – Id of the road from where getting the waypoint.  
        - `lane_id` (_int_) – Id of the lane to get the waypoint.  
        - `s` (_float_) – Specify the length from the road start.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Map.get_topology"></a>**<font color="#7fb800">get_topology</font>**(<font color="#00a6ed">**self**</font>)  
It provides a minimal graph of the topology of the current OpenDRIVE file. It is constituted by a list of pairs of waypoints, where the first waypoint is the origin and the second one is the destination. It can be loaded into [NetworkX](https://networkx.github.io/). A valid output could be: `[ (w0, w1), (w0, w2), (w1, w3), (w2, w3), (w0, w4) ]`.  
    - **Return:** _list(tuple([carla.Waypoint](#carla.Waypoint), [carla.Waypoint](#carla.Waypoint)))_  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#7fb800">generate_waypoints</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints positioned on the center of the lanes all over the map with an approximate distance between them.  
    - **Parameters:**
        - `distance` (_float_) – Approximate distance between the waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#7fb800">transform_to_geolocation</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**path**</font>)  
Saves the .xodr OpenDRIVE file of the current map to disk.  
    - **Parameters:**
        - `path` – Path where the file will be saved.  
- <a name="carla.Map.to_opendrive"></a>**<font color="#7fb800">to_opendrive</font>**(<font color="#00a6ed">**self**</font>)  
Returns the .xodr OpenDRIVe file of the current map as string.  
    - **Return:** _str_  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#7fb800">transform_to_geolocation</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>)  
Converts a given `location`, a point in the simulation, to a [carla.GeoLocation](#carla.GeoLocation), which represents world coordinates. The geographical location of the map is defined inside OpenDRIVE within the tag <b><georeference></b>.  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
    - **Return:** _[carla.GeoLocation](#carla.GeoLocation)_  
- <a name="carla.Map.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.ObstacleDetectionEvent<a name="carla.ObstacleDetectionEvent"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Obstacle detection sensor data.  

<h3>Instance Variables</h3>
- <a name="carla.ObstacleDetectionEvent.actor"></a>**<font color="#f8805a">actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get "self" actor. Actor that measured the collision.  
- <a name="carla.ObstacleDetectionEvent.other_actor"></a>**<font color="#f8805a">other_actor</font>** (_[carla.Actor](#carla.Actor)_)  
Get the actor to which we collided.  
- <a name="carla.ObstacleDetectionEvent.distance"></a>**<font color="#f8805a">distance</font>** (_float_)  
Get obstacle distance.  

<h3>Methods</h3>
- <a name="carla.ObstacleDetectionEvent.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RadarDetection<a name="carla.RadarDetection"></a>
Data contained by a [carla.RadarMeasurement](#carla.RadarMeasurement). Represents an object detection produced by the Radar sensor.  

<h3>Instance Variables</h3>
- <a name="carla.RadarDetection.velocity"></a>**<font color="#f8805a">velocity</font>** (_float_)  
The velocity of the detected object towards the sensor in meters per second.  
- <a name="carla.RadarDetection.azimuth"></a>**<font color="#f8805a">azimuth</font>** (_float_)  
Azimuth angle of the detection in radians.  
- <a name="carla.RadarDetection.altitude"></a>**<font color="#f8805a">altitude</font>** (_float_)  
Altitude angle of the detection in radians.  
- <a name="carla.RadarDetection.depth"></a>**<font color="#f8805a">depth</font>** (_float_)  
Distance in meters from the sensor to the detection position.  

<h3>Methods</h3>
- <a name="carla.RadarDetection.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.RadarMeasurement<a name="carla.RadarMeasurement"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.SensorData](#carla.SensorData)_</b></small></div></p><p>Measurement produced by a Radar. Consists of an array of [carla.RadarDetection](#carla.RadarDetection).  

<h3>Instance Variables</h3>
- <a name="carla.RadarMeasurement.raw_data"></a>**<font color="#f8805a">raw_data</font>** (_bytes_)  
List of [carla.RadarDetection](#carla.RadarDetection).  

<h3>Methods</h3>
- <a name="carla.RadarMeasurement.get_detection_count"></a>**<font color="#7fb800">get_detection_count</font>**(<font color="#00a6ed">**self**</font>)  
Retrieve the number of [carla.RadarDetection](#carla.RadarDetection) that are generated.  
- <a name="carla.RadarMeasurement.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.RadarMeasurement.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.RadarMeasurement.__getitem__"></a>**<font color="#7fb800">\__getitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
- <a name="carla.RadarMeasurement.__setitem__"></a>**<font color="#7fb800">\__setitem__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pos**</font>, <font color="#00a6ed">**detection**</font>)  
    - **Parameters:**
        - `pos` (_int_)  
        - `detection` (_[carla.RadarDetection](#carla.RadarDetection)_)  
- <a name="carla.RadarMeasurement.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Rotation<a name="carla.Rotation"></a>
Class that represents a 3D rotation. All rotation angles are stored in degrees.

![UE4_Rotation](https://d26ilriwvtzlb.cloudfront.net/8/83/BRMC_9.jpg) _Unreal Engine's standard (from [UE4 docs](https://wiki.unrealengine.com/Blueprint_Rotating_Movement_Component))_.  

<h3>Instance Variables</h3>
- <a name="carla.Rotation.pitch"></a>**<font color="#f8805a">pitch</font>** (_float_)  
Rotation about Y-axis.  
- <a name="carla.Rotation.yaw"></a>**<font color="#f8805a">yaw</font>** (_float_)  
Rotation about Z-axis.  
- <a name="carla.Rotation.roll"></a>**<font color="#f8805a">roll</font>** (_float_)  
Rotation about X-axis.  

<h3>Methods</h3>
- <a name="carla.Rotation.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**pitch**=0.0</font>, <font color="#00a6ed">**yaw**=0.0</font>, <font color="#00a6ed">**roll**=0.0</font>)  
    - **Parameters:**
        - `pitch` (_float_)  
        - `yaw` (_float_)  
        - `roll` (_float_)  
- <a name="carla.Rotation.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes a forward vector using the current rotation.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Rotation.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Rotation](#carla.Rotation)_)  
- <a name="carla.Rotation.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Rotation](#carla.Rotation)_)  
- <a name="carla.Rotation.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Sensor<a name="carla.Sensor"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.Actor](#carla.Actor)_</b></small></div></p><p>A sensor actor.  

<h3>Instance Variables</h3>
- <a name="carla.Sensor.is_listening"></a>**<font color="#f8805a">is_listening</font>** (_boolean_)  
Is true if the sensor is listening for data.  

<h3>Methods</h3>
- <a name="carla.Sensor.listen"></a>**<font color="#7fb800">listen</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)  
    - **Parameters:**
        - `callback` (_function_) – Register a callback to be executed each time a new measurement is received. The callback must accept a single argument containing the sensor data; the type of this object varies depending on the type of sensor, but they all derive from [carla.SensorData](#carla.SensorData).  
- <a name="carla.Sensor.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)  
Stops listening for data.  
- <a name="carla.Sensor.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.SensorData<a name="carla.SensorData"></a>
Base class for all the objects containing data generated by a sensor.  

<h3>Instance Variables</h3>
- <a name="carla.SensorData.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
Frame count when the data was generated.  
- <a name="carla.SensorData.timestamp"></a>**<font color="#f8805a">timestamp</font>** (_float_)  
Simulation-time when the data was generated.  
- <a name="carla.SensorData.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Sensor's transform when the data was generated.  

---

## carla.Timestamp<a name="carla.Timestamp"></a>
Class that contains Timestamp simulated data.  

<h3>Instance Variables</h3>
- <a name="carla.Timestamp.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
The number of frames elapsed since the simulator was launched.  
- <a name="carla.Timestamp.elapsed_seconds"></a>**<font color="#f8805a">elapsed_seconds</font>** (_float_)  
Simulated seconds elapsed since the beginning of the current episode.  
- <a name="carla.Timestamp.delta_seconds"></a>**<font color="#f8805a">delta_seconds</font>** (_float_)  
Simulated seconds elapsed since the previous frame.  
- <a name="carla.Timestamp.platform_timestamp"></a>**<font color="#f8805a">platform_timestamp</font>** (_float_)  
Time-stamp of the frame at which this measurement was taken, in seconds as given by the OS.  

<h3>Methods</h3>
- <a name="carla.Timestamp.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**frame**</font>, <font color="#00a6ed">**elapsed_seconds**</font>, <font color="#00a6ed">**delta_seconds**</font>, <font color="#00a6ed">**platform_timestamp**</font>)  
    - **Parameters:**
        - `frame` (_int_)  
        - `elapsed_seconds` (_float_)  
        - `delta_seconds` (_float_)  
        - `platform_timestamp` (_float_)  
- <a name="carla.Timestamp.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.Timestamp.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.Timestamp.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.TrafficLight<a name="carla.TrafficLight"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.TrafficSign](#carla.TrafficSign)_</b></small></div></p><p>A traffic light actor. Check out this [`recipe`](../python_cookbook/#traffic-lights-recipe)!  

<h3>Instance Variables</h3>
- <a name="carla.TrafficLight.state"></a>**<font color="#f8805a">state</font>** (_[carla.TrafficLightState](#carla.TrafficLightState)_)  
Current traffic light state.  

<h3>Methods</h3>
- <a name="carla.TrafficLight.set_state"></a>**<font color="#7fb800">set_state</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**state**</font>)  
Sets a given state to a traffic light actor.  
    - **Parameters:**
        - `state` (_[carla.TrafficLightState](#carla.TrafficLightState)_)  
- <a name="carla.TrafficLight.get_state"></a>**<font color="#7fb800">get_state</font>**(<font color="#00a6ed">**self**</font>)  
Returns the current state of the traffic light.  
    - **Return:** _[carla.TrafficLightState](#carla.TrafficLightState)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.set_green_time"></a>**<font color="#7fb800">set_green_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**green_time**</font>)  
    - **Parameters:**
        - `green_time` (_float_) – Sets a given time (in seconds) to the green state to be active.  
- <a name="carla.TrafficLight.get_green_time"></a>**<font color="#7fb800">get_green_time</font>**(<font color="#00a6ed">**self**</font>)  
Returns the current time set for the green light to be active.  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.set_yellow_time"></a>**<font color="#7fb800">set_yellow_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**yellow_time**</font>)  
Sets a given time (in seconds) to the yellow state to be active.  
    - **Parameters:**
        - `yellow_time` (_float_)  
- <a name="carla.TrafficLight.get_yellow_time"></a>**<font color="#7fb800">get_yellow_time</font>**(<font color="#00a6ed">**self**</font>)  
Returns the current time set for the yellow light to be active.  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.set_red_time"></a>**<font color="#7fb800">set_red_time</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**red_time**</font>)  
Sets a given time (in seconds) to the red state to be active.  
    - **Parameters:**
        - `red_time` (_float_)  
- <a name="carla.TrafficLight.get_red_time"></a>**<font color="#7fb800">get_red_time</font>**(<font color="#00a6ed">**self**</font>)  
Returns the current time set for the red light to be active.  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.get_elapsed_time"></a>**<font color="#7fb800">get_elapsed_time</font>**(<font color="#00a6ed">**self**</font>)  
Returns the current countdown of the state of a traffic light.  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.freeze"></a>**<font color="#7fb800">freeze</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**freeze**</font>)  
Stops the traffic light at its current state.  
    - **Parameters:**
        - `freeze` (_bool_)  
- <a name="carla.TrafficLight.is_frozen"></a>**<font color="#7fb800">is_frozen</font>**(<font color="#00a6ed">**self**</font>)  
Returns `True` if a traffic light is frozen.  
    - **Return:** _bool_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.TrafficLight.get_pole_index"></a>**<font color="#7fb800">get_pole_index</font>**(<font color="#00a6ed">**self**</font>)  
Returns the index of the pole in the traffic light group.  
    - **Return:** _int_  
- <a name="carla.TrafficLight.get_group_traffic_lights"></a>**<font color="#7fb800">get_group_traffic_lights</font>**(<font color="#00a6ed">**self**</font>)  
Returns all traffic lights in the group this one belongs to.  
    - **Return:** _list([carla.TrafficLight](#carla.TrafficLight))_  
    - **Note:** <font color="#8E8E8E">_This function calls the simulator.
_</font>  
- <a name="carla.TrafficLight.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.TrafficLightState<a name="carla.TrafficLightState"></a>
All possible states for traffic lights. Check out this [`recipe`](../python_cookbook/#traffic-lights-recipe)!  

<h3>Instance Variables</h3>
- <a name="carla.TrafficLightState.Red"></a>**<font color="#f8805a">Red</font>**  
- <a name="carla.TrafficLightState.Yellow"></a>**<font color="#f8805a">Yellow</font>**  
- <a name="carla.TrafficLightState.Green"></a>**<font color="#f8805a">Green</font>**  
- <a name="carla.TrafficLightState.Off"></a>**<font color="#f8805a">Off</font>**  
- <a name="carla.TrafficLightState.Unknown"></a>**<font color="#f8805a">Unknown</font>**  

---

## carla.TrafficSign<a name="carla.TrafficSign"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.Actor](#carla.Actor)_</b></small></div></p><p>A traffic sign actor.  

<h3>Instance Variables</h3>
- <a name="carla.TrafficSign.trigger_volume"></a>**<font color="#f8805a">trigger_volume</font>**  
A [carla.BoundingBox](#carla.BoundingBox) situated near a traffic sign where the [carla.Actor](#carla.Actor) who is inside can know about its state.  

---

## carla.Transform<a name="carla.Transform"></a>
Class that defines a transformation without scaling.  

<h3>Instance Variables</h3>
- <a name="carla.Transform.location"></a>**<font color="#f8805a">location</font>** (_[carla.Location](#carla.Location)_)  
- <a name="carla.Transform.rotation"></a>**<font color="#f8805a">rotation</font>** (_[carla.Rotation](#carla.Rotation)_)  

<h3>Methods</h3>
- <a name="carla.Transform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**location**</font>, <font color="#00a6ed">**rotation**</font>)  
    - **Parameters:**
        - `location` (_[carla.Location](#carla.Location)_)  
        - `rotation` (_[carla.Rotation](#carla.Rotation)_)  
- <a name="carla.Transform.transform"></a>**<font color="#7fb800">transform</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**in_point**</font>)  
Transform a 3D point using the current transformation.  
    - **Parameters:**
        - `in_point` (_[carla.Location](#carla.Location)_) – Location in the space to which the transformation will be applied.  
- <a name="carla.Transform.get_forward_vector"></a>**<font color="#7fb800">get_forward_vector</font>**(<font color="#00a6ed">**self**</font>)  
Computes a forward vector using the rotation of the current transformation.  
    - **Return:** _[carla.Vector3D](#carla.Vector3D)_  
- <a name="carla.Transform.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Transform](#carla.Transform)_)  
- <a name="carla.Transform.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Transform](#carla.Transform)_)  
- <a name="carla.Transform.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Vector2D<a name="carla.Vector2D"></a>
Vector 2D helper class.  

<h3>Instance Variables</h3>
- <a name="carla.Vector2D.x"></a>**<font color="#f8805a">x</font>** (_float_)  
- <a name="carla.Vector2D.y"></a>**<font color="#f8805a">y</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Vector2D.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
- <a name="carla.Vector2D.__add__"></a>**<font color="#7fb800">\__add__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__mul__"></a>**<font color="#7fb800">\__mul__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__sub__"></a>**<font color="#7fb800">\__sub__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__truediv__"></a>**<font color="#7fb800">\__truediv__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector2D](#carla.Vector2D)_)  
- <a name="carla.Vector2D.__self__"></a>**<font color="#7fb800">\__self__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Vector3D<a name="carla.Vector3D"></a>
Vector 3D helper class.  

<h3>Instance Variables</h3>
- <a name="carla.Vector3D.x"></a>**<font color="#f8805a">x</font>** (_float_)  
- <a name="carla.Vector3D.y"></a>**<font color="#f8805a">y</font>** (_float_)  
- <a name="carla.Vector3D.z"></a>**<font color="#f8805a">z</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.Vector3D.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**x**=0.0</font>, <font color="#00a6ed">**y**=0.0</font>, <font color="#00a6ed">**z**=0.0</font>)  
    - **Parameters:**
        - `x` (_float_)  
        - `y` (_float_)  
        - `z` (_float_)  
- <a name="carla.Vector3D.__add__"></a>**<font color="#7fb800">\__add__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__mul__"></a>**<font color="#7fb800">\__mul__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__sub__"></a>**<font color="#7fb800">\__sub__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__truediv__"></a>**<font color="#7fb800">\__truediv__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.Vector3D.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Vehicle<a name="carla.Vehicle"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.Actor](#carla.Actor)_</b></small></div></p><p>A vehicle actor.  

<h3>Instance Variables</h3>
- <a name="carla.Vehicle.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
The vehicle's bounding box.  

<h3>Methods</h3>
- <a name="carla.Vehicle.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Apply control to this vehicle. The control will take effect on next tick.  
    - **Parameters:**
        - `control` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.Vehicle.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
Returns the control last applied to this vehicle.  
    - **Return:** _[carla.VehicleControl](#carla.VehicleControl)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.apply_physics_control"></a>**<font color="#7fb800">apply_physics_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**physics_control**</font>)  
Apply physics control to this vehicle. The control will take effect on the next tick.  
    - **Parameters:**
        - `physics_control` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.Vehicle.get_physics_control"></a>**<font color="#7fb800">get_physics_control</font>**(<font color="#00a6ed">**self**</font>)  
Returns the physics control last applied to this vehicle.  
    - **Return:** _[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_  
    - **Warning:** <font color="#ED2F2F">_This function does call the simulator to retrieve the value._</font>  
- <a name="carla.Vehicle.set_autopilot"></a>**<font color="#7fb800">set_autopilot</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**enabled**=True</font>)  
Switch on/off this vehicle's server-side autopilot.  
    - **Parameters:**
        - `enabled` (_bool_)  
- <a name="carla.Vehicle.get_speed_limit"></a>**<font color="#7fb800">get_speed_limit</font>**(<font color="#00a6ed">**self**</font>)  
Returns the speed limit currently affecting this vehicle. Note that the speed limit is only updated when passing by a speed limit signal, right after spawning a vehicle it might not reflect the actual speed limit of the current road.  
    - **Return:** _float_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.get_traffic_light_state"></a>**<font color="#7fb800">get_traffic_light_state</font>**(<font color="#00a6ed">**self**</font>)  
Returns the state of the traffic light currently affecting this vehicle. If no traffic light is currently affecting the vehicle, return Green.  
    - **Return:** _[carla.TrafficLightState](#carla.TrafficLightState)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.is_at_traffic_light"></a>**<font color="#7fb800">is_at_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
Returns whether a traffic light is affecting this vehicle.  
    - **Return:** _bool_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Vehicle.get_traffic_light"></a>**<font color="#7fb800">get_traffic_light</font>**(<font color="#00a6ed">**self**</font>)  
Retrieve the traffic light actor currently affecting this vehicle.  
    - **Return:** _[carla.TrafficLight](#carla.TrafficLight)_  
- <a name="carla.Vehicle.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _str_  

---

## carla.VehicleControl<a name="carla.VehicleControl"></a>
VehicleControl is used for controlling the basic movement of a vehicle.  

<h3>Instance Variables</h3>
- <a name="carla.VehicleControl.throttle"></a>**<font color="#f8805a">throttle</font>** (_float_)  
A scalar value to control the vehicle throttle [0.0, 1.0].  
- <a name="carla.VehicleControl.steer"></a>**<font color="#f8805a">steer</font>** (_float_)  
A scalar value to control the vehicle steering [-1.0, 1.0].  
- <a name="carla.VehicleControl.brake"></a>**<font color="#f8805a">brake</font>** (_float_)  
A scalar value to control the vehicle brake [0.0, 1.0].  
- <a name="carla.VehicleControl.hand_brake"></a>**<font color="#f8805a">hand_brake</font>** (_bool_)  
If true, hand brake will be used.  
- <a name="carla.VehicleControl.reverse"></a>**<font color="#f8805a">reverse</font>** (_bool_)  
If true, the vehicle will move reverse.  
- <a name="carla.VehicleControl.manual_gear_shift"></a>**<font color="#f8805a">manual_gear_shift</font>** (_bool_)  
If true, the vehicle will be controlled by changing gears manually.  
- <a name="carla.VehicleControl.gear"></a>**<font color="#f8805a">gear</font>** (_int_)  
Controls the gear value of the vehicle.  

<h3>Methods</h3>
- <a name="carla.VehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**throttle**=0.0</font>, <font color="#00a6ed">**steer**=0.0</font>, <font color="#00a6ed">**brake**=0.0</font>, <font color="#00a6ed">**hand_brake**=True</font>, <font color="#00a6ed">**reverse**=True</font>, <font color="#00a6ed">**manual_gear_shift**=True</font>, <font color="#00a6ed">**gear**=0</font>)  
VehicleControl constructor.  
    - **Parameters:**
        - `throttle` (_float_)  
        - `steer` (_float_)  
        - `brake` (_float_)  
        - `hand_brake` (_bool_)  
        - `reverse` (_bool_)  
        - `manual_gear_shift` (_bool_)  
        - `gear` (_int_)  
- <a name="carla.VehicleControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.VehicleControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehicleControl](#carla.VehicleControl)_)  
- <a name="carla.VehicleControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.VehicleLightState<a name="carla.VehicleLightState"></a>
Class that recaps the state of the lights of a vehicle, these can be used as a flags. E.g: `VehicleLightState.HighBeam & VehicleLightState.Brake` will return `True` when both are active. Lights are off by default in any situation and should be managed by the user via script. The blinkers blink automatically. _Warning: Right now, not all vehicles have been prepared to work with this functionality, this will be added to all of them in later updates_.  

<h3>Instance Variables</h3>
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
- <a name="carla.VehicleLightState.Any"></a>**<font color="#f8805a">Any</font>**  
All lights on.  

---

## carla.VehiclePhysicsControl<a name="carla.VehiclePhysicsControl"></a>
VehiclePhysicsControl is used for controlling the physics parameters of a vehicle.  

<h3>Instance Variables</h3>
- <a name="carla.VehiclePhysicsControl.torque_curve"></a>**<font color="#f8805a">torque_curve</font>** (_list([carla.Vector2D](#carla.Vector2D))_)  
Curve that indicates the torque measured in Nm for a specific RPM of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.max_rpm"></a>**<font color="#f8805a">max_rpm</font>** (_float_)  
The maximum RPM of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.moi"></a>**<font color="#f8805a">moi</font>** (_float_)  
The moment of inertia of the vehicle's engine.  
- <a name="carla.VehiclePhysicsControl.damping_rate_full_throttle"></a>**<font color="#f8805a">damping_rate_full_throttle</font>** (_float_)  
Damping rate when the throttle is maximum.  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_engaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_engaged</font>** (_float_)  
Damping rate when the throttle is zero with clutch engaged.  
- <a name="carla.VehiclePhysicsControl.damping_rate_zero_throttle_clutch_disengaged"></a>**<font color="#f8805a">damping_rate_zero_throttle_clutch_disengaged</font>** (_float_)  
Damping rate when the throttle is zero with clutch disengaged.  
- <a name="carla.VehiclePhysicsControl.use_gear_autobox"></a>**<font color="#f8805a">use_gear_autobox</font>** (_bool_)  
If true, the vehicle will have an automatic transmission.  
- <a name="carla.VehiclePhysicsControl.gear_switch_time"></a>**<font color="#f8805a">gear_switch_time</font>** (_float_)  
Switching time between gears.  
- <a name="carla.VehiclePhysicsControl.clutch_strength"></a>**<font color="#f8805a">clutch_strength</font>** (_float_)  
The clutch strength of the vehicle. Measured in Kgm^2/s.  
- <a name="carla.VehiclePhysicsControl.final_ratio"></a>**<font color="#f8805a">final_ratio</font>** (_float_)  
The fixed ratio from transmission to wheels.  
- <a name="carla.VehiclePhysicsControl.forward_gears"></a>**<font color="#f8805a">forward_gears</font>** (_list([carla.GearPhysicsControl](#carla.GearPhysicsControl))_)  
List of GearPhysicsControl objects.  
- <a name="carla.VehiclePhysicsControl.mass"></a>**<font color="#f8805a">mass</font>** (_float_)  
The mass of the vehicle measured in Kg.  
- <a name="carla.VehiclePhysicsControl.drag_coefficient"></a>**<font color="#f8805a">drag_coefficient</font>** (_float_)  
Drag coefficient of the vehicle's chassis.  
- <a name="carla.VehiclePhysicsControl.center_of_mass"></a>**<font color="#f8805a">center_of_mass</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
The center of mass of the vehicle.  
- <a name="carla.VehiclePhysicsControl.steering_curve"></a>**<font color="#f8805a">steering_curve</font>** (_list([carla.Vector2D](#carla.Vector2D))_)  
Curve that indicates the maximum steering for a specific forward speed.  
- <a name="carla.VehiclePhysicsControl.wheels"></a>**<font color="#f8805a">wheels</font>** (_list([carla.WheelPhysicsControl](#carla.WheelPhysicsControl))_)  
List of [carla.WheelPhysicsControl](#carla.WheelPhysicsControl) objects. This list should have 4 elements, where index 0 corresponds to the front left wheel, index 1 corresponds to the front right wheel, index 2 corresponds to the back left wheel and index 3 corresponds to the back right wheel. For 2 wheeled vehicles, set the same values for both front and back wheels.  

<h3>Methods</h3>
- <a name="carla.VehiclePhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**torque_curve**=[[0.0, 500.0], [5000.0, 500.0]]</font>, <font color="#00a6ed">**max_rpm**=5000.0</font>, <font color="#00a6ed">**moi**=1.0</font>, <font color="#00a6ed">**damping_rate_full_throttle**=0.15</font>, <font color="#00a6ed">**damping_rate_zero_throttle_clutch_engaged**=2.0</font>, <font color="#00a6ed">**damping_rate_zero_throttle_clutch_disengaged**=0.35</font>, <font color="#00a6ed">**use_gear_autobox**=True</font>, <font color="#00a6ed">**gear_switch_time**=0.5</font>, <font color="#00a6ed">**clutch_strength**=10.0</font>, <font color="#00a6ed">**final_ratio**=4.0</font>, <font color="#00a6ed">**forward_gears**=list()</font>, <font color="#00a6ed">**mass**=1000.0</font>, <font color="#00a6ed">**drag_coefficient**=0.3</font>, <font color="#00a6ed">**center_of_mass**=[0.0, 0.0, 0.0]</font>, <font color="#00a6ed">**steering_curve**=[[0.0, 1.0], [10.0, 0.5]]</font>, <font color="#00a6ed">**wheels**=list()</font>)  
VehiclePhysicsControl constructor.  
    - **Parameters:**
        - `torque_curve` (_list([carla.Vector2D](#carla.Vector2D))_)  
        - `max_rpm` (_float_)  
        - `moi` (_float_)  
        - `damping_rate_full_throttle` (_float_)  
        - `damping_rate_zero_throttle_clutch_engaged` (_float_)  
        - `damping_rate_zero_throttle_clutch_disengaged` (_float_)  
        - `use_gear_autobox` (_bool_)  
        - `gear_switch_time` (_float_)  
        - `clutch_strength` (_float_)  
        - `final_ratio` (_float_)  
        - `forward_gears` (_list([carla.GearPhysicsControl](#carla.GearPhysicsControl))_)  
        - `drag_coefficient` (_float_)  
        - `center_of_mass` (_[carla.Vector3D](#carla.Vector3D)_)  
        - `steering_curve` (_[carla.Vector2D](#carla.Vector2D)_)  
        - `wheels` (_list([carla.WheelPhysicsControl](#carla.WheelPhysicsControl))_)  
- <a name="carla.VehiclePhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.VehiclePhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.VehiclePhysicsControl](#carla.VehiclePhysicsControl)_)  
- <a name="carla.VehiclePhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Walker<a name="carla.Walker"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.Actor](#carla.Actor)_</b></small></div></p><p>A walking actor, pedestrian.  

<h3>Instance Variables</h3>
- <a name="carla.Walker.bounding_box"></a>**<font color="#f8805a">bounding_box</font>** (_[carla.BoundingBox](#carla.BoundingBox)_)  
The walker's bounding box.  

<h3>Methods</h3>
- <a name="carla.Walker.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Apply control to this walker.  
    - **Parameters:**
        - `control` (_[carla.WalkerControl](#carla.WalkerControl)_)  
    - **Note:** <font color="#8E8E8E">_The control will take effect on the next tick.
_</font>  
- <a name="carla.Walker.apply_control"></a>**<font color="#7fb800">apply_control</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**control**</font>)  
Apply bone control to this walker.  
    - **Parameters:**
        - `control` (_[carla.WalkerBoneControl](#carla.WalkerBoneControl)_)  
    - **Note:** <font color="#8E8E8E">_The control will take effect on the next tick.
_</font>  
- <a name="carla.Walker.get_control"></a>**<font color="#7fb800">get_control</font>**(<font color="#00a6ed">**self**</font>)  
Returns the control last applied to this walker.  
    - **Return:** _[carla.WalkerControl](#carla.WalkerControl)_  
    - **Note:** <font color="#8E8E8E">_This function does not call the simulator, it returns the data received in the last tick.
_</font>  
- <a name="carla.Walker.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _str_  

---

## carla.WalkerAIController<a name="carla.WalkerAIController"></a>
<div style="padding-left:30px;margin-top:-20px"><small><b>Inherited from _[carla.Actor](#carla.Actor)_</b></small></div></p><p>Class used for controlling the automation of a pedestrian.  

<h3>Methods</h3>
- <a name="carla.WalkerAIController.start"></a>**<font color="#7fb800">start</font>**(<font color="#00a6ed">**self**</font>)  
Initializes walker controller.  
- <a name="carla.WalkerAIController.stop"></a>**<font color="#7fb800">stop</font>**(<font color="#00a6ed">**self**</font>)  
Stops walker controller.  
- <a name="carla.WalkerAIController.go_to_location"></a>**<font color="#7fb800">go_to_location</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**destination**</font>)  
Sets the destination that the pedestrian will reach.  
    - **Parameters:**
        - `destination` (_[carla.Location](#carla.Location)_)  
- <a name="carla.WalkerAIController.set_max_speed"></a>**<font color="#7fb800">set_max_speed</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**speed**=1.4</font>)  
Sets the speed of the pedestrian.  
    - **Parameters:**
        - `speed` (_float_) – Speed is in m/s.  
- <a name="carla.WalkerAIController.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerBoneControl<a name="carla.WalkerBoneControl"></a>
Class used for controlling the skeleton of a walker. See [walker bone control](walker_bone_control.md).  

<h3>Instance Variables</h3>
- <a name="carla.WalkerBoneControl.bone_transforms"></a>**<font color="#f8805a">bone_transforms</font>** (_list([name,transform])_)  
List of pairs where the first value is the bone name and the second value is the bone transform.  

<h3>Methods</h3>
- <a name="carla.WalkerBoneControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**list(name,transform)**</font>)  
    - **Parameters:**
        - `list(name,transform)` (_tuple_)  
- <a name="carla.WalkerBoneControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WalkerControl<a name="carla.WalkerControl"></a>
WalkerControl is used for controlling the basic movement of a walker.  

<h3>Instance Variables</h3>
- <a name="carla.WalkerControl.direction"></a>**<font color="#f8805a">direction</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Vector that controls the direction of the walker.  
- <a name="carla.WalkerControl.speed"></a>**<font color="#f8805a">speed</font>** (_float_)  
A scalar value to control the walker speed.  
- <a name="carla.WalkerControl.jump"></a>**<font color="#f8805a">jump</font>** (_bool_)  
If true, the walker will perform a jump.  

<h3>Methods</h3>
- <a name="carla.WalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**direction**=[1.0, 0.0, 0.0]</font>, <font color="#00a6ed">**speed**=0.0</font>, <font color="#00a6ed">**jump**=False</font>)  
WalkerControl constructor.  
    - **Parameters:**
        - `direction` (_[carla.Vector3D](#carla.Vector3D)_)  
        - `speed` (_float_)  
        - `jump` (_bool_)  
- <a name="carla.WalkerControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WalkerControl](#carla.WalkerControl)_)  
- <a name="carla.WalkerControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WalkerControl](#carla.WalkerControl)_)  
- <a name="carla.WalkerControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.Waypoint<a name="carla.Waypoint"></a>
Waypoints in CARLA are described as 3D directed points. They store a certain [carla.Transform](#carla.Transform) which locates the waypoint in a road and orientates it according to the lane. They also store the road information belonging to said point regarding its lane and lane markings. All of this information is retrieved as provided by the OpenDRIVE file.  

<h3>Instance Variables</h3>
- <a name="carla.Waypoint.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The identificator is generated using a hash combination of the <b>road</b>, <b>section</b>, <b>lane</b> and <b>s</b> values that correspond to said point in the OpenDRIVE geometry. The <b>s</b> precision is set to 2 centimeters, so 2 waypoints closer than 2 centimeters in the same road, section and lane, will have the same identificator.  
- <a name="carla.Waypoint.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Position and orientation of the waypoint according to the current lane information. This data is computed the first time it is accessed. It is not created right away in order to ease computing costs when lots of waypoints are created but their specific transform is not needed.  
- <a name="carla.Waypoint.road_id"></a>**<font color="#f8805a">road_id</font>** (_int_)  
OpenDRIVE road's id.  
- <a name="carla.Waypoint.section_id"></a>**<font color="#f8805a">section_id</font>** (_int_)  
OpenDRIVE section's id, based on the order that they are originally defined.  
- <a name="carla.Waypoint.lane_id"></a>**<font color="#f8805a">lane_id</font>** (_int_)  
OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the current lane with respect to the road. For more information refer to OpenDRIVE [documentation](http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf#page=20).  
- <a name="carla.Waypoint.s"></a>**<font color="#f8805a">s</font>** (_float_)  
OpenDRIVE <b>s</b> value of the current position.  
- <a name="carla.Waypoint.is_junction"></a>**<font color="#f8805a">is_junction</font>** (_bool_)  
<b>True</b> if the current Waypoint is on a junction as defined by OpenDRIVE.  
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

<h3>Methods</h3>
- <a name="carla.Waypoint.get_left_lane"></a>**<font color="#7fb800">get_left_lane</font>**(<font color="#00a6ed">**self**</font>)  
Generates a Waypoint at the center of the left lane based on the direction of the current Waypoint, regardless if the lane change is allowed in this location.
Can return <b>None</b> if the lane does not exist.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Waypoint.get_right_lane"></a>**<font color="#7fb800">get_right_lane</font>**(<font color="#00a6ed">**self**</font>)  
Generates a waypoint at the center of the right lane based on the direction of the current waypoint, regardless if the lane change is allowed in this location.
Can return <b>None</b> if the lane does not exist.  
    - **Return:** _[carla.Waypoint](#carla.Waypoint)_  
- <a name="carla.Waypoint.next"></a>**<font color="#7fb800">next</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints at a certain approximate `distance` from the current one. It takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option.
The list may be empty if the road ends before the specified distance, for instance, a lane ending with the only option of incorporating to another road.  
    - **Parameters:**
        - `distance` (_float_) – The approximate distance where to get the next waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.next_until_lane_end"></a>**<font color="#7fb800">next_until_lane_end</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints from this to the end of the lane separated by a certain `distance`.  
    - **Parameters:**
        - `distance` (_float_) – The approximate distance between waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.previous"></a>**<font color="#7fb800">previous</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
This method does not return the waypoint previously visited by an actor, but a list of waypoints at an approximate `distance` but in the opposite direction of the lane. Similarly to **<font color="#7fb800">next()</font>**, it takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option.
The list may be empty if the road ends before the specified distance, for instance, a lane ending with the only option of incorporating to another road.  
    - **Parameters:**
        - `distance` (_float_) – The approximate distance where to get the previous waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.previous_until_lane_start"></a>**<font color="#7fb800">previous_until_lane_start</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**distance**</font>)  
Returns a list of waypoints from this to the start of the lane separated by a certain `distance`.  
    - **Parameters:**
        - `distance` (_float_) – The approximate distance between waypoints.  
    - **Return:** _list([carla.Waypoint](#carla.Waypoint))_  
- <a name="carla.Waypoint.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WeatherParameters<a name="carla.WeatherParameters"></a>
WeatherParameters class is used for requesting and changing the lighting and weather conditions inside the world.  

<h3>Instance Variables</h3>
- <a name="carla.WeatherParameters.cloudiness"></a>**<font color="#f8805a">cloudiness</font>** (_float_)  
Weather cloudiness. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.precipitation"></a>**<font color="#f8805a">precipitation</font>** (_float_)  
Precipitation amount for controlling rain intensity. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.precipitation_deposits"></a>**<font color="#f8805a">precipitation_deposits</font>** (_float_)  
Precipitation deposits for controlling the area of puddles on roads. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.wind_intensity"></a>**<font color="#f8805a">wind_intensity</font>** (_float_)  
Wind intensity, it affects the clouds moving speed, the raindrop direction, and vegetation. This doesn't affect the car physics. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.fog_density"></a>**<font color="#f8805a">fog_density</font>** (_float_)  
Fog density. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.fog_distance"></a>**<font color="#f8805a">fog_distance</font>** (_float_)  
Fog start distance (in meters). Values range from 0 to infinite.  
- <a name="carla.WeatherParameters.wetness"></a>**<font color="#f8805a">wetness</font>** (_float_)  
Wetness intensity. It only affects the RGB camera sensor. Values range from 0 to 100.  
- <a name="carla.WeatherParameters.sun_azimuth_angle"></a>**<font color="#f8805a">sun_azimuth_angle</font>** (_float_)  
The azimuth angle of the sun in degrees. Values range from 0 to 360 (degrees).  
- <a name="carla.WeatherParameters.sun_altitude_angle"></a>**<font color="#f8805a">sun_altitude_angle</font>** (_float_)  
Altitude angle of the sun in degrees. Values range from -90 to 90 (where 0 degrees is the horizon).  

<h3>Methods</h3>
- <a name="carla.WeatherParameters.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**cloudiness**=0.0</font>, <font color="#00a6ed">**precipitation**=0.0</font>, <font color="#00a6ed">**precipitation_deposits**=0.0</font>, <font color="#00a6ed">**wind_intensity**=0.0</font>, <font color="#00a6ed">**sun_azimuth_angle**=0.0</font>, <font color="#00a6ed">**sun_altitude_angle**=0.0</font>)  
WeatherParameters constructor.  
    - **Parameters:**
        - `cloudiness` (_float_)  
        - `precipitation` (_float_)  
        - `precipitation_deposits` (_float_)  
        - `wind_intensity` (_float_)  
        - `sun_azimuth_angle` (_float_)  
        - `sun_altitude_angle` (_float_)  
- <a name="carla.WeatherParameters.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
Returns True if `self` and `other` are equal.  
    - **Return:** _bool_  
- <a name="carla.WeatherParameters.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
Returns True if `self` and `other` are not equal.  
    - **Return:** _bool_  
- <a name="carla.WeatherParameters.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WheelPhysicsControl<a name="carla.WheelPhysicsControl"></a>
WheelPhysicsControl is used for controlling the physics parameters of a vehicle's wheel.  

<h3>Instance Variables</h3>
- <a name="carla.WheelPhysicsControl.tire_friction"></a>**<font color="#f8805a">tire_friction</font>** (_float_)  
A scalar value that indicates the friction of the wheel.  
- <a name="carla.WheelPhysicsControl.damping_rate"></a>**<font color="#f8805a">damping_rate</font>** (_float_)  
The damping rate of the wheel.  
- <a name="carla.WheelPhysicsControl.max_steer_angle"></a>**<font color="#f8805a">max_steer_angle</font>** (_float_)  
The maximum angle in degrees that the wheel can steer.  
- <a name="carla.WheelPhysicsControl.radius"></a>**<font color="#f8805a">radius</font>** (_float_)  
The radius of the wheel in centimeters.  
- <a name="carla.WheelPhysicsControl.max_brake_torque"></a>**<font color="#f8805a">max_brake_torque</font>** (_float_)  
The maximum brake torque in Nm.  
- <a name="carla.WheelPhysicsControl.max_handbrake_torque"></a>**<font color="#f8805a">max_handbrake_torque</font>** (_float_)  
The maximum handbrake torque in Nm.  
- <a name="carla.WheelPhysicsControl.position"></a>**<font color="#f8805a">position</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
World position of the wheel. Note that it is a read-only parameter.  

<h3>Methods</h3>
- <a name="carla.WheelPhysicsControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**tire_friction**=2.0</font>, <font color="#00a6ed">**damping_rate**=0.25</font>, <font color="#00a6ed">**max_steer_angle**=70.0</font>, <font color="#00a6ed">**radius**=30.0</font>, <font color="#00a6ed">**max_brake_torque**=1500.0</font>, <font color="#00a6ed">**max_handbrake_torque**=3000.0</font>, <font color="#00a6ed">**position**=(0.0,0.0,0.0)</font>)  
WheelPhysicsControl constructor.  
    - **Parameters:**
        - `tire_friction` (_float_)  
        - `damping_rate` (_float_)  
        - `max_steer_angle` (_float_)  
        - `radius` (_float_)  
        - `max_brake_torque` (_float_)  
        - `max_handbrake_torque` (_float_)  
        - `position` (_[carla.Vector3D](#carla.Vector3D)_)  
- <a name="carla.WheelPhysicsControl.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WheelPhysicsControl](#carla.WheelPhysicsControl)_)  
- <a name="carla.WheelPhysicsControl.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WheelPhysicsControl](#carla.WheelPhysicsControl)_)  
- <a name="carla.WheelPhysicsControl.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.World<a name="carla.World"></a>
Class that contains the current loaded map.  

<h3>Instance Variables</h3>
- <a name="carla.World.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The id of the episode associated with this world.  
- <a name="carla.World.debug"></a>**<font color="#f8805a">debug</font>** (_[carla.DebugHelper](#carla.DebugHelper)_)  

<h3>Methods</h3>
- <a name="carla.World.get_blueprint_library"></a>**<font color="#7fb800">get_blueprint_library</font>**(<font color="#00a6ed">**self**</font>)  
Return the list of blueprints available in this world. These blueprints can be used to spawn actors into the world.  
    - **Return:** _[carla.BlueprintLibrary](#carla.BlueprintLibrary)_  
- <a name="carla.World.get_map"></a>**<font color="#7fb800">get_map</font>**(<font color="#00a6ed">**self**</font>)  
Return the map that describes this world.  
    - **Return:** _[carla.Map](#carla.Map)_  
- <a name="carla.World.get_spectator"></a>**<font color="#7fb800">get_spectator</font>**(<font color="#00a6ed">**self**</font>)  
Return the spectator actor. The spectator controls the view in the simulator window.  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.get_settings"></a>**<font color="#7fb800">get_settings</font>**(<font color="#00a6ed">**self**</font>)  
    - **Return:** _[carla.WorldSettings](#carla.WorldSettings)_  
- <a name="carla.World.apply_settings"></a>**<font color="#7fb800">apply_settings</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**world_settings**</font>)  
Returns the id of the frame when the settings took effect.  
    - **Parameters:**
        - `world_settings` (_[carla.WorldSettings](#carla.WorldSettings)_)  
    - **Return:** _int_  
- <a name="carla.World.get_weather"></a>**<font color="#7fb800">get_weather</font>**(<font color="#00a6ed">**self**</font>)  
Retrieve the weather parameters currently active in the world.  
    - **Return:** _[carla.WeatherParameters](#carla.WeatherParameters)_  
- <a name="carla.World.set_weather"></a>**<font color="#7fb800">set_weather</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**weather**</font>)  
Change the weather in the simulation.  
    - **Parameters:**
        - `weather` (_[carla.WeatherParameters](#carla.WeatherParameters)_)  
- <a name="carla.World.get_snapshot"></a>**<font color="#7fb800">get_snapshot</font>**(<font color="#00a6ed">**self**</font>)  
Return a snapshot of the world at this moment.  
    - **Return:** _[carla.WorldSnapshot](#carla.WorldSnapshot)_  
- <a name="carla.World.get_actor"></a>**<font color="#7fb800">get_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Find actor by id, return None if not found.  
    - **Parameters:**
        - `actor_id` (_int_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.get_random_location_from_navigation"></a>**<font color="#7fb800">get_random_location_from_navigation</font>**(<font color="#00a6ed">**self**</font>)  
Retrieve a random location to be used as a destination for walkers in [carla.WalkerAIController.go_to_location](#carla.WalkerAIController.go_to_location). See [`spawn_npc.py`](https://github.com/carla-simulator/carla/blob/e73ad54d182e743b50690ca00f1709b08b16528c/PythonAPI/examples/spawn_npc.py#L179) for an example.  
    - **Return:** _[carla.Location](#carla.Location)_  
- <a name="carla.World.get_actors"></a>**<font color="#7fb800">get_actors</font>**(<font color="#00a6ed">**self**</font>)  
By default it returns a list with every actor present in the world. _A list of ids can be used as a parameter_.  
    - **Return:** _[carla.ActorList](#carla.ActorList)_  
- <a name="carla.World.spawn_actor"></a>**<font color="#7fb800">spawn_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**attach_to**=None</font>, <font color="#00a6ed">**attachment**=Rigid</font>)  
Spawn an actor into the world based on the blueprint provided at transform. If a parent is provided, the actor is attached to parent.  
    - **Parameters:**
        - `blueprint` (_[carla.BlueprintLibrary](#carla.BlueprintLibrary)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_) – If attached to parent, transform acts like a relative_transform to the parent actor.  
        - `attach_to` (_[carla.Actor](#carla.Actor)_)  
        - `attachment` (_[carla.AttachmentType](#carla.AttachmentType)_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.try_spawn_actor"></a>**<font color="#7fb800">try_spawn_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**blueprint**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**attach_to**=None</font>, <font color="#00a6ed">**attachment**=Rigid</font>)  
Same as SpawnActor but return none on failure instead of throwing an exception.  
    - **Parameters:**
        - `blueprint` (_[carla.BlueprintLibrary](#carla.BlueprintLibrary)_)  
        - `transform` (_[carla.Transform](#carla.Transform)_) – If attached to parent, transform acts like a relative_transform to the parent actor.  
        - `attach_to` (_[carla.Actor](#carla.Actor)_)  
        - `attachment` (_[carla.AttachmentType](#carla.AttachmentType)_)  
    - **Return:** _[carla.Actor](#carla.Actor)_  
- <a name="carla.World.wait_for_tick"></a>**<font color="#7fb800">wait_for_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**seconds**=10.0</font>)  
Block calling thread until a world tick is received.  
    - **Parameters:**
        - `seconds` (_float_)  
    - **Return:** _[carla.WorldSnapshot](#carla.WorldSnapshot)_  
- <a name="carla.World.on_tick"></a>**<font color="#7fb800">on_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback**</font>)  
Returns the ID of the callback so it can be removed with `remove_on_tick`.  
    - **Parameters:**
        - `callback` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_)  
    - **Return:** _int_  
- <a name="carla.World.remove_on_tick"></a>**<font color="#7fb800">remove_on_tick</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**callback_id**</font>)  
Removes on tick callbacks.  
- <a name="carla.World.tick"></a>**<font color="#7fb800">tick</font>**(<font color="#00a6ed">**self**</font>)  
Synchronizes with the simulator and returns the id of the newly started frame (only has effect on synchronous mode).  
    - **Return:** _int_  
- <a name="carla.World.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WorldSettings<a name="carla.WorldSettings"></a>
Class that provides access to modifiable world settings. Check it out in our [section](../configuring_the_simulation/).  

<h3>Instance Variables</h3>
- <a name="carla.WorldSettings.synchronous_mode"></a>**<font color="#f8805a">synchronous_mode</font>** (_bool_)  
- <a name="carla.WorldSettings.no_rendering_mode"></a>**<font color="#f8805a">no_rendering_mode</font>** (_bool_)  
- <a name="carla.WorldSettings.fixed_delta_seconds"></a>**<font color="#f8805a">fixed_delta_seconds</font>** (_float_)  

<h3>Methods</h3>
- <a name="carla.WorldSettings.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**synchronous_mode**=False</font>, <font color="#00a6ed">**no_rendering_mode**=False</font>, <font color="#00a6ed">**fixed_delta_seconds**=0.0</font>)  
    - **Parameters:**
        - `synchronous_mode` (_bool_)  
        - `no_rendering_mode` (_bool_)  
        - `fixed_delta_seconds` (_float_)  
- <a name="carla.WorldSettings.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.WorldSettings.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.Timestamp](#carla.Timestamp)_)  
- <a name="carla.WorldSettings.__str__"></a>**<font color="#7fb800">\__str__</font>**(<font color="#00a6ed">**self**</font>)  

---

## carla.WorldSnapshot<a name="carla.WorldSnapshot"></a>
Class that represents the state of every actor in the simulation at a single frame.  

<h3>Instance Variables</h3>
- <a name="carla.WorldSnapshot.id"></a>**<font color="#f8805a">id</font>** (_int_)  
The WorldSnapshot's identifier.  
- <a name="carla.WorldSnapshot.frame"></a>**<font color="#f8805a">frame</font>** (_int_)  
Frame number.  
- <a name="carla.WorldSnapshot.timestamp"></a>**<font color="#f8805a">timestamp</font>** (_[carla.Timestamp](#carla.Timestamp)_)  
Timestamp simulated data.  

<h3>Methods</h3>
- <a name="carla.WorldSnapshot.has_actor"></a>**<font color="#7fb800">has_actor</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Check if an actor is present in this snapshot.  
    - **Parameters:**
        - `actor_id` (_int_)  
- <a name="carla.WorldSnapshot.find"></a>**<font color="#7fb800">find</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor_id**</font>)  
Find an ActorSnapshot by id, return None if the actor is not found.  
    - **Parameters:**
        - `actor_id` (_int_)  
- <a name="carla.WorldSnapshot.__len__"></a>**<font color="#7fb800">\__len__</font>**(<font color="#00a6ed">**self**</font>)  
Return number of [carla.ActorSnapshot](#carla.ActorSnapshot) present in this [carla.WorldSnapshot](#carla.WorldSnapshot).  
    - **Return:** _int_  
- <a name="carla.WorldSnapshot.__iter__"></a>**<font color="#7fb800">\__iter__</font>**(<font color="#00a6ed">**self**</font>)  
- <a name="carla.WorldSnapshot.__eq__"></a>**<font color="#7fb800">\__eq__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_)  
- <a name="carla.WorldSnapshot.__ne__"></a>**<font color="#7fb800">\__ne__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**other**</font>)  
    - **Parameters:**
        - `other` (_[carla.WorldSnapshot](#carla.WorldSnapshot)_)  
- <a name="carla.WorldSnapshot.__self__"></a>**<font color="#7fb800">\__self__</font>**(<font color="#00a6ed">**self**</font>)  

---

## command.ApplyAngularVelocity<a name="command.ApplyAngularVelocity"></a>
Set the actor's angular velocity.  

<h3>Instance Variables</h3>
- <a name="command.ApplyAngularVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyAngularVelocity.angular_velocity"></a>**<font color="#f8805a">angular_velocity</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
The 3D angular velocity that will be applied to the actor.  

<h3>Methods</h3>
- <a name="command.ApplyAngularVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**angular_velocity**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `angular_velocity` (_[carla.Vector3D](#carla.Vector3D)_)  

---

## command.ApplyImpulse<a name="command.ApplyImpulse"></a>
Adds impulse to the actor.  

<h3>Instance Variables</h3>
- <a name="command.ApplyImpulse.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyImpulse.impulse"></a>**<font color="#f8805a">impulse</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
Impulse applied to the actor.  

<h3>Methods</h3>
- <a name="command.ApplyImpulse.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**impulse**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `impulse` (_[carla.Vector3D](#carla.Vector3D)_)  

---

## command.ApplyTransform<a name="command.ApplyTransform"></a>
Sets a new transform to the actor.  

<h3>Instance Variables</h3>
- <a name="command.ApplyTransform.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyTransform.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transformation to be applied.  

<h3>Methods</h3>
- <a name="command.ApplyTransform.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**transform**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  

---

## command.ApplyVehicleControl<a name="command.ApplyVehicleControl"></a>
Apply control to the vehicle.  

<h3>Instance Variables</h3>
- <a name="command.ApplyVehicleControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Vehicle actor affected by the command.  
- <a name="command.ApplyVehicleControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.VehicleControl](#carla.VehicleControl)_)  
Vehicle control to be applied.  

<h3>Methods</h3>
- <a name="command.ApplyVehicleControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `control` (_[carla.VehicleControl](#carla.VehicleControl)_)  

---

## command.ApplyVelocity<a name="command.ApplyVelocity"></a>
Sets actor's velocity.  

<h3>Instance Variables</h3>
- <a name="command.ApplyVelocity.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.ApplyVelocity.velocity"></a>**<font color="#f8805a">velocity</font>** (_[carla.Vector3D](#carla.Vector3D)_)  
The 3D velocity applied to the actor.  

<h3>Methods</h3>
- <a name="command.ApplyVelocity.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**velocity**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `velocity` (_[carla.Vector3D](#carla.Vector3D)_)  

---

## command.ApplyWalkerControl<a name="command.ApplyWalkerControl"></a>
Apply control to the walker.  

<h3>Instance Variables</h3>
- <a name="command.ApplyWalkerControl.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Walker actor affected by the command.  
- <a name="command.ApplyWalkerControl.control"></a>**<font color="#f8805a">control</font>** (_[carla.VehicleControl](#carla.VehicleControl)_)  
Walker control to be applied.  

<h3>Methods</h3>
- <a name="command.ApplyWalkerControl.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**control**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `control` (_[carla.WalkerControl](#carla.WalkerControl)_)  

---

## command.ApplyWalkerState<a name="command.ApplyWalkerState"></a>
Apply a state to the walker actor.  

<h3>Instance Variables</h3>
- <a name="command.ApplyWalkerState.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Walker actor affected by the command.  
- <a name="command.ApplyWalkerState.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transform to be applied.  
- <a name="command.ApplyWalkerState.speed"></a>**<font color="#f8805a">speed</font>** (_float_)  
Speed to be applied.  

<h3>Methods</h3>
- <a name="command.ApplyWalkerState.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**transform**</font>, <font color="#00a6ed">**speed**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `transform` (_[carla.Transform](#carla.Transform)_)  
        - `speed` (_float_)  

---

## command.DestroyActor<a name="command.DestroyActor"></a>
Tell the simulator to destroy this Actor, and return whether the actor was successfully destroyed. It has no effect if the Actor was already successfully destroyed.  

<h3>Instance Variables</h3>
- <a name="command.DestroyActor.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  

<h3>Methods</h3>
- <a name="command.DestroyActor.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  

---

## command.Response<a name="command.Response"></a>
Execution result of a command, contains either an error string or an actor ID, depending on whether or not the command succeeded. See [carla.Client.apply_batch_sync](#carla.Client.apply_batch_sync).  

<h3>Instance Variables</h3>
- <a name="command.Response.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.Response.error"></a>**<font color="#f8805a">error</font>** (_str_)  

<h3>Methods</h3>
- <a name="command.Response.has_error"></a>**<font color="#7fb800">has_error</font>**(<font color="#00a6ed">**self**</font>)  

---

## command.SetAutopilot<a name="command.SetAutopilot"></a>
Switch on/off vehicle's server-side autopilot.  

<h3>Instance Variables</h3>
- <a name="command.SetAutopilot.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor that is affected by the command.  
- <a name="command.SetAutopilot.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  
If the autopilot is enabled or not.  

<h3>Methods</h3>
- <a name="command.SetAutopilot.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `enabled` (_bool_)  

---

## command.SetSimulatePhysics<a name="command.SetSimulatePhysics"></a>
Whether an actor will be affected by physics or not.  

<h3>Instance Variables</h3>
- <a name="command.SetSimulatePhysics.actor_id"></a>**<font color="#f8805a">actor_id</font>** (_int_)  
Actor affected by the command.  
- <a name="command.SetSimulatePhysics.enabled"></a>**<font color="#f8805a">enabled</font>** (_bool_)  
If physics will affect the actor.  

<h3>Methods</h3>
- <a name="command.SetSimulatePhysics.__init__"></a>**<font color="#7fb800">\__init__</font>**(<font color="#00a6ed">**self**</font>, <font color="#00a6ed">**actor**</font>, <font color="#00a6ed">**enabled**</font>)  
    - **Parameters:**
        - `actor` (_[carla.Actor](#carla.Actor) or int_)  
        - `enabled` (_bool_)  

---

## command.SpawnActor<a name="command.SpawnActor"></a>
Spawn an actor into the world based on the blueprint provided and the transform. If a parent is provided, the actor is attached to parent.  

<h3>Instance Variables</h3>
- <a name="command.SpawnActor.transform"></a>**<font color="#f8805a">transform</font>** (_[carla.Transform](#carla.Transform)_)  
Transform to be applied.  
- <a name="command.SpawnActor.parent_id"></a>**<font color="#f8805a">parent_id</font>** (_int_)  
Parent's actor id.  

<h3>Methods</h3>
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
Link another command to be executed right after.  
    - **Parameters:**
        - `command` (_[carla.Command](#carla.Command)_) – CommandType.  

---