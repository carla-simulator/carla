---
title: Developer/Implementation Notes/Drawing in sumo-gui
permalink: /Developer/Implementation_Notes/Drawing_in_sumo-gui/
---

# Basic

Each network object has a type defined in GUIGlObjectTypes.h (i.e.
GLO_EDGE, GLO_JUNCTION, GL_VEHICLE, ...). Objects are maintained in
three places:

- SUMORTree: a structure for finding objects based on their location
  (used for efficient drawing)
- GUIGlObjectStorage: a structure for retrieving objects based on
  their numerical id and for locking objects to prevent concurrent
  access
- type specific containers (GUIEdges for example are maintained in
  MSEdge::dictionary)

  !!! note
      This also applies to netedit

# Layers

As all objects are drawn in the order they are stored within the rtree,
no z-ordering is done before. Due to this, the objects must be draw in
"layers". This is realised by using glTranslated. Each object type is
also an integer value (enum type) which is used as the layer in which to
draw objects of this type. Since GLO_VEHICLE has the integer value of
256 it is drawn on top of GLO_LANE which has the lower value 2. Some
objects consists of multiple layers themselves (i.e. Busstops). In this
case increments of 0.1 are used which should not interfere with objects
of different types.

As a general rule each drawing routine should use glPushMatrix at the
beginning, set the layer with glTranslated(0,0,getType()) and use
glPopMatrix when finished. Top level captions (junction names, edge
names, ...) are drawn at the layer GLO_MAX. A list of all predefined
layers is present in
**<SUMO_HOME\>**/src/utils/gui/globjects/GUIGlObjectTypes.h.

The layer information stored in [Polygons and
POIs](../../Simulation/Shapes.md) is used in the same way as the
GUIGlObjectType (glTranslate(0,0,getLayer()). This makes it possible to
place polygons anywhere in the drawing stack, allowing the visual
emulation of tunnels and other road-obscuring features.