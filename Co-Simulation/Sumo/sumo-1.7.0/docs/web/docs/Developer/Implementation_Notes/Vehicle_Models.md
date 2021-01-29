---
title: Developer/Implementation Notes/Vehicle Models
permalink: /Developer/Implementation_Notes/Vehicle_Models/
---

!!! caution
    Please note that this is a working document... No user documentation!

# Implementing new Car-Following Models

A car-following model in SUMO is an implementation of the abstract class
MSCFModel (microsim/cfmodels/MSCFModel.h).

The following steps must be taken to add a new model:

1.  define a new model name in utils/common/SUMOXMLDefinitions.h and
    .cpp
2.  define model parameters in
    utild/xml/SUMOVehicleParserHelper::getAllowedCFModelAttrs() (define
    new attributes in SUMOXMLDefinitions if necessary)
3.  add a new class to microsim/cfmodels (i.e. by copying one of the
    existing classes there)
4.  add this class to the build files (i.e.
    microsim/cfmodels/Makefile.am when building on Linux)
5.  add code that selects the new class based on the new name in
    microsim/MSVehicleType::build()

If your model needs persistent state you need to derive a class to hold
it from MSCFModel::VehicleVariables. See
microsim/cfmodels/MSCFModel_IDM as an example.

# Defining Vehicle Types

Before starting to hack, we should decide how vehicle type shall be
represented. Some facts and needs (unsorted):

- <font color="green">ok</font> Both the simulation and the routing
  modules must be able to parse vehicle type definitions
- It would be nice to allow a validation against XML Schemata
- At least two sub-types must be able to be defined: car-following
  model and lane-changing model (maybe the lane-changing model even
  splits into a navigational and a tactical part)
- <font color="green">ok</font> Parameter should have default values
- <font color="green">ok</font> a default vehicle type must exist
- Models differ in parameter sets

## Possible Descriptions

<table>
<thead>
<tr class="header">
<th><p>Index</p></th>
<th><p>Sample</p></th>
<th><p>pros/cons</p></th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td><p>A</p></td>
<td><p>&lt;vtype cfModel="xxx" xxxParam1="..." xxxParam2="..." ...</p></td>
<td><ul>
<li>(-) hard to verify - parameter change in dependence to the value of "cfModel"</li>
</ul></td>
</tr>
<tr class="even">
<td><p>B</p></td>
<td><p>&lt;vtype-XXX xxxParam1="..." xxxParam2="..." ...</p></td>
<td><ul>
<li>(-) large number of different elements needed</li>
</ul></td>
</tr>
<tr class="odd">
<td><p>C</p></td>
<td><p>&lt;cfmodel id="#id" model="xxx" param1="..." param2="..." .../&gt;&lt;vtype cfModelId="#id" .../&gt;</p></td>
<td><ul>
<li>(+) straight forward; many combinations possible</li>
<li>(+) is similar to how every vehicle references its vtype</li>
<li>(-) hard to verify - parameter change in dependence to the value of "cfModel"</li>
</ul></td>
</tr>
<tr class="even">
<td><p>D</p></td>
<td><p>&lt;vtype cfModel="xxx" param1="..." param2="..." ...&gt;</p></td>
<td><ul>
<li>(--) parameter sets differ between models</li>
<li>(-) hard to verify - parameter change in dependence to the value of "cfModel"</li>
</ul></td>
</tr>
<tr class="odd">
<td><p>E</p></td>
<td><p>&lt;vtype ...&gt;&lt;cfmodel model="xxx" param1="..." param2="..." .../&gt;</vtype></p></td>
<td><ul>
<li>(+) straight forward</li>
<li>(-) hard to verify - parameter change in dependence to the value of "cfModel"</li>
<li><strong>1 vote (Mayer)</strong></li>
</ul></td>
</tr>
<tr class="even">
<td><p>F</p></td>
<td><p>&lt;vtype ...&gt;&lt;cfmodel-XXX model="xxx" param1="..." param2="..." .../&gt;</vtype></p></td>
<td><ul>
<li>(+) straight forward</li>
<li>(-) large number of different elements needed</li>
<li><strong>2 votes (Behrisch, Krajzewicz)</strong></li>
</ul></td>
</tr>
</tbody>
</table>

## (currently) Chosen Description

After some talks, the following description of vehicle types was chosen:

    <vtype id="..." ...>
        <carFollowing-Krauss ...
        <laneChanging-DK2002 ...
    </vtype>

`vtype` will contain parameters which can
neither be counted to the lane-changing nor the car-following model,
such as the vehicle type's color, the width of the vehicle etc. which
are (currently) used for visualisation only. Also, the vehicle's length
and other parameter which are not only used by one of the models are
stored herein.

The embedded `carFollowing-Krauss`-Element in
this example describes the car-following model (Krauss in this case),
the `laneChanging-DK2002` the lane-changing
model (Daniel Krajzewicz's from 2002 :-) ), each with their own
parameter.

The known `vtype`-definition is wanted to be
kept. In this case, the Krauss-model stays chosen per default.

!!! note
    Probably, it will not be possible to validate this against a schema.

# Car-following Model Interface

Currently, it is assumed that the following methods should be
re-implemented for each model; the "generic" column should identify
those which may be same across models:

| method              | generic?  | description                                                                     |
| ------------------- | --------- | ------------------------------------------------------------------------------- |
| ffeV                | no        |                                                                                 |
| ffeS                | no        |                                                                                 |
| maxNextSpeed        | no        | internal SUMO-tweak                                                             |
| brakeGap            | maybe     | should depend on maximum deceleration (a rather common value) and physics, only |
| approachingBrakeGap | maybe     | almost same as brakeGap, only not incorporating the driver's reaction time      |
| interactionGap      | no        |                                                                                 |
| hasSafeGap          | no        |                                                                                 |
| safeEmitGap         | no        |                                                                                 |
| dawdle              | no        |                                                                                 |
| decelAbility        | rather no |                                                                                 |

## Loading and Parsing Vehicle Types

As already implemented for vehicles, a new intermediate structure
{{SUMO}}/src/utils/common/SUMOVTypeParameter which contains vehicle type
descriptions was added. When reading XML-definitions, both routers and
the simulation use the additionally implemented helper methods located
in {{SUMO}}/src/utils/xml/SUMOVehicleParserHelper for filling this structure.

SUMOVTypeParameter has member variables for those vehicle type
parameters which are assumed to be neither part of the car-following
model nor part of the lane-change model: *id*, *length*, *maxSpeed*,
*defaultProbability*, *speedFactor*, *speedDev*, *emissionClass*,
*color*, *vehicleClass*, *width*, *offset*, *shape*. These values are
initialised with defaults. When reading values, SUMOVehicleParserHelper
stores the information about which value was set from the XML
description in *SUMOVTypeParameter::setParameter*. This allows to save
(pass) only set values when writing the definitions back to a file -
routers need this.

Parameters of the car-following model are saved into a map,
*SUMOVTypeParameter::cfParameter*, the model's name to
*SUMOVTypeParameter::cfModel*. Note that lane-changing model parameter
handling is not implemented. The map contains only those parameter of
the model which were given in the read XML file; no defaults are
inserted at this time. In order to allow processing of old
vtype-definitions, parameter stored directly within the vtype-element
for the Krauss car-following model are stored within this map, too.

The routers consume the SUMOVTypeParameter class directly - making
ROVehicleType unnecessary. The simulation uses the new method
MSVehicleType::build for building the described MSVehicleType/MSCFModel
combination. Missing defaults are set within this method for obtaining
complete vehicle type/model descriptions.