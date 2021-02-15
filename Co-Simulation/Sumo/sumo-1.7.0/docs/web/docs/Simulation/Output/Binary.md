---
title: Simulation/Output/Binary
permalink: /Simulation/Output/Binary/
---

All [SUMO Applications](../../index.md#application_manuals)
([sumo](../../sumo.md), [netconvert](../../netconvert.md),...) can
save all its outputs including networks and routes in SUMO Binary XML
(.sbx) and also read the corresponding files as input. This output
format is used automatically whenever an output file name with extension
*.sbx* is specified. In order to read these files they must also have
the *.sbx* extension (for all other extensions, XML is assumed). The
format is XML and TraCI inspired and aims to be self contained (can be
read without additional information from a network etc.).

## Data types

Every data item is preceeded by a single byte describing the type which
is one of the following:

- Byte (single unsigned byte, also used for boolean values)
- Integer (32bit, signed, endianness see below)
- Double (standard double precision floating point)
- String (32bit integer encoding length (n), followed by n unsigned
  chars 7bit-ASCII; UTF8 may be implemented later)
- List (32bit integer encoding length (n), followed by n entries each
  having a possibly different type stated up front)
- XML-Element-Start (two bytes referring to the element list)
- XML-Element-End (no data, just an indicator)
- XML-Attribut (two bytes referring to the attribute list, see below)
- Edge (32bit integer referring to the edge list, currently unused)
- Lane (32bit integer referring to the edge list, followed by a single
  byte encoding the index, currently unused)
- Position2D (2 doubles)
- Position3D (3 doubles)
- Color (4 bytes)
- Node-Type (single byte referring to the respective list, see below)
- Edge-Function (single byte referring to the respective list, see
  below)
- Route (32bit integer encoding length (n), followed by bineary
  encoded route, see below)
- Scaled Integer (32bit int, fixed point float with two decimals)
- Scaled Position2D (2 32bit int, analogue Position2D fixed point
  float with two decimals)
- Scaled Position3D (3 32bit int, analogue Position3D fixed point
  float with two decimals)

The change from version 1 to version 2 consisted of an "upgrade" of
XML-Element-Start and XML-Attribut to two bytes (least significant
first) whereas the explicit reference to the XML-element is now omitted
from XML-Element-End. Please note that encoding shapes binary may
increase file size because small nets only need 7 bytes per coordinate
when truncated to two decimals. Regarding endianness / byte order:
Opposed to TraCI we use platform byte order for easier usage with the
FileHelper-functions.

## Format

### Header

Every file starts with a single byte SUMO Binary XML version number,
followed by a string containing the SUMO version. Before SUMO 0.29.0 the
version number was 1, starting with 0.29.0 it is 2. After that there are

- a list of strings containing all XML elements
- a list of strings containing all XML attributes
- a list of strings containing all node types
- a list of strings containing all edge functions
- a list of strings containing all edges
- a list of connection lists which themselves contain ints as
  reference into the edge list

The two last lists may be empty but if they are, the usage of the edge,
lane and route types are not allowed (the respective entries need to be
strings)

### "real content"

- simple XML oriented style with the following PseudoBNF:
  - Simple = Byte | Integer | Double | String | List | Edge | Lane |
    Position2D | Position3D | Color | Node-Type | Edge-Function |
    Route
  - XMLContent = XML-Element-Start (XML-Attribut Simple)\*
    XMLContent\* XML-ELement-End
- no comments or whitespace, no character data

### Route encoding

Routes, or more precisely lists of consecutive (in the sense of being
connected in the network) edges, can be compressed most efficiently. If
the route is connected every edge is encoded as the successor index to
the previous edge in the connection list from the header. This is done
using as few bits as possible (e.g. if no edge has more than 16
successors, 4 bits suffice). For unconnected edge lists all edges are
simply stored by their index.

## Remarks

The element, attribute and type lists are usually ordered the same way
they are in SUMOXMLDefinitions.cpp, but this should be tested at parse
(currently it is not\!). This is also true for the edge and connection
lists.

There is a script converting the binary format back to plain XML:
<SUMO\>/tools/xml/binary2plain.py.

The current implementation guesses from the file extension (.sbx)
whether binary output is wished. It is currently not possible to write
binary to sockets or stdout.