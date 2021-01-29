---
title: Developer/How To/Extend Vehicles
permalink: /Developer/How_To/Extend_Vehicles/
---

This brief document describes how the vehicle class can be extended. We
will do this based on extending the vehicle by the capacity to transport
persons.

# Basics

The main vehicle parameter are stored in
src/utils/vehicle/SUMOVehicleParameter. At first, we extend the class
(.h) by two further parameter:

```
/// @brief The vehicle's capacity (persons)
unsigned int personCapacity;

/// @brief The number of persons in the vehicle
unsigned int personNumber;
```

We also assign default values for those in the constructor (.cpp):

```
... personCapacity(0), personNumber(0), ...
```

Our vehicles now have the attribute, and we can use it within the
simulation as well as within the routing modules.

# I/O

The vehicle parser stores whether an information is given or not, for
writing it back if necessary. We have to define constants (bitset) which
describe whether the value was given (.h):

```
const int VEHPARS_PERSON_CAPACITY_SET = 2 << 14;
const int VEHPARS_PERSONS_NUMBER_SET = 2 << 15;
```

## Writing a Vehicle

We extend the vehicle writing method stored in void
SUMOVehicleParameter::writeAs(...) const by the new attributes:

```
if (wasSet(VEHPARS_PERSON_CAPACITY_SET)) {
 dev.writeAttr(SUMO_ATTR_PERSON_CAPACITY, personCapacity);
}
if (wasSet(VEHPARS_PERSON_NUMBER_SET)) {
 dev.writeAttr(SUMO_ATTR_PERSON_NUMBER, personNumber);
}
```

You may have noticed that we use the enumeration for two XML-attributes,
here: "SUMO_ATTR_PERSON_CAPACITY" and "SUMO_ATTR_PERSON_NUMBER".
They have to be defined in src/utils/xml/SUMOXMLDefinitions. In
SUMOXMLDefinitions.h we have to extend SumoXMLAttr:

```
SUMO_ATTR_PERSON_CAPACITY,
SUMO_ATTR_PERSON_NUMBER,
```

In SUMOXMLDefinitions.cpp we have to extend SumoXMLAttr, too:

```
{ "personCapacity", SUMO_ATTR_PERSON_CAPACITY },
{ "personNumber",   SUMO_ATTR_PERSON_NUMBER },
```

The given strings will be later used as XML-attributes.

## Loading

We then extend the vehicle parsing method. It is located in
src/utils/xml/SUMOVehicleParserHelper. We have to add the parsing of the
new attributes in void
SUMOVehicleParserHelper::parseCommonAttributes(...)

```
// parse person number
if (attrs.hasAttribute(SUMO_ATTR_PERSON_NUMBER)) {
 ret->setParameter |= VEHPARS_PERSON_NUMBER_SET;
 ret->personNumber = attrs.getIntReporting(SUMO_ATTR_PERSON_NUMBER, 0, ok);
}
// parse person capacity
if (attrs.hasAttribute(SUMO_ATTR_PERSON_CAPACITY)) {
 ret->setParameter |= VEHPARS_PERSON_CAPACITY_SET;
 ret->personCapacity = attrs.getIntReporting(SUMO_ATTR_PERSON_CAPACITY, 0, ok);
}
```

# Usage