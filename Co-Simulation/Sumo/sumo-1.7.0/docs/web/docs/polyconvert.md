---
title: polyconvert
permalink: /polyconvert/
---

# From 30.000 feet

**polyconvert** imports geometrical shapes (polygons or points of
interest) from different sources, converts them to a representation that
may be visualized using [sumo-gui](sumo-gui.md).

- **Purpose:** Polygon and POI import, conversion, and projection
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):** polygons or pois
- **Output:** SUMO-shape file
- **Programming Language:** C++

## Features

- Imports OSM, VISUM, Elmar, XML, ArcView shape files
- per-type import settings
- projections using a given proj.4-definition or via a matching
  network
- Writes [simulation shape files](Simulation/Shapes.md) usable
  within [sumo-gui](sumo-gui.md) and [sumo](sumo.md)

# Usage Description

**polyconvert** is able to import shapes from
different file types. Normally, for importing data of a certain type,
the type name is used as option name and the value indicates the
position of the file. So

```
polyconvert --visum mynet.net -o converted.poi.xml
```

imports from a VISUM-net file.

## Options

You may use a XML schema definition file for setting up a polyconvert
configuration:
[polyconvertConfiguration.xsd](http://sumo.dlr.de/xsd/polyconvertConfiguration.xsd).

### Configuration

All applications of the **SUMO**-suite handle configuration options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Configuration
Files](Basics/Using_the_Command_Line_Applications.md#configuration_files).

| Option | Description |
|--------|-------------|
| **-c** {{DT_FILE}}<br> **--configuration-file** {{DT_FILE}} | Loads the named config on startup |
| **-C** {{DT_FILE}}<br> **--save-configuration** {{DT_FILE}} | Saves current configuration into FILE |
| **--save-template** {{DT_FILE}} | Saves a configuration template (empty) into FILE |
| **--save-schema** {{DT_FILE}} | Saves the configuration schema into FILE |
| **--save-commented** {{DT_BOOL}} | Adds comments to saved template, configuration, or schema; *default:* **false** |

### Input

**polyconvert** is capable to apply different
attributes to the imported shapes in dependence of their "type". Not all
imported formats have a type information. When using shape files, for
example, all instances of an artifact type are normally stored in a
distinct shape file.

| Option | Description |
|--------|-------------|
| **-n** {{DT_FILE}}<br> **--net-file** {{DT_FILE}} | Loads SUMO-network FILE as reference to offset and projection |
| **--dlr-navteq-poly-files** {{DT_FILE}} | Reads polygons from FILE assuming they're coded in DLR-Navteq (Elmar)-format |
| **--dlr-navteq-poi-files** {{DT_FILE}} | Reads pois from FILE+ assuming they're coded in DLR-Navteq (Elmar)-format |
| **--visum-files** {{DT_FILE}} | Reads polygons from FILE assuming it's a Visum-net |
| **--xml-files** {{DT_FILE}} | Reads pois and shapes from FILE assuming they're coded in XML |
| **--osm-files** {{DT_FILE}} | Reads pois from FILE+ assuming they're coded in OSM |
| **--osm.keep-full-type** {{DT_BOOL}} | The type will be made of the key-value - pair; *default:* **false** |
| **--osm.use-name** {{DT_BOOL}} | The id will be set from the given 'name' attribute; *default:* **false** |
| **--osm.merge-relations** {{DT_FLOAT}} | If FLOAT >= 0, assemble one polygon from all ways of a relation if they all connect with gaps below FLOAT; *default:* **-1** |
| **--shapefile-prefixes** {{DT_FILE}} | Reads shapes from shapefiles FILE+ |
| **--shapefile.guess-projection** {{DT_BOOL}} | Guesses the shapefile's projection; *default:* **false** |
| **--shapefile.traditional-axis-mapping** {{DT_BOOL}} | Use traditional axis order (lon, lat); *default:* **false** |
| **--shapefile.id-column** {{DT_STR}} | Defines in which column the id can be found |
| **--shapefile.type-columns** {{DT_STR[]}} | Defines which columns form the type id (comma separated list) |
| **--shapefile.use-running-id** {{DT_BOOL}} | A running number will be used as id; *default:* **false** |
| **--shapefile.add-param** {{DT_BOOL}} | Extract all additional columns as params; *default:* **false** |
| **--shapefile.fill** {{DT_STR}} | [auto|true|false]. Forces the 'fill' status to the given value. Default 'auto' tries to determine it from the data type |
| **--type-file** {{DT_FILE}} | Reads types from FILE |

### Output

All imported shapes that have not been discarded are written into a file
which has to be defined using **--output** {{DT_FILE}}.

| Option | Description |
|--------|-------------|
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **-o** {{DT_FILE}}<br> **--output-file** {{DT_FILE}} | Write generated polygons/pois to FILE |
| **--dlr-tdp-output** {{DT_FILE}} | Write generated polygons/pois to a dlr-tdp file with the given prefix |

### Projection

One of the major uses of **polyconvert** is to
apply a projection on the read shapes. Normally, one wants the shapes to
be aligned in accordance to a previously imported road network. In this
case, the network should be given using **--net-file** {{DT_FILE}}. But it is also possible to use
a different projection. In any case, if the read coordinates shall be
changed, **--use-projection** must be given.

| Option | Description |
|--------|-------------|
| **--simple-projection** {{DT_BOOL}} | Uses a simple method for projection; *default:* **false** |
| **--proj.scale** {{DT_FLOAT}} | Scaling factor for input coordinates; *default:* **1** |
| **--proj.rotate** {{DT_FLOAT}} | Rotation (clockwise degrees) for input coordinates; *default:* **0** |
| **--proj.utm** {{DT_BOOL}} | Determine the UTM zone (for a universal transversal mercator projection based on the WGS84 ellipsoid); *default:* **false** |
| **--proj.dhdn** {{DT_BOOL}} | Determine the DHDN zone (for a transversal mercator projection based on the bessel ellipsoid, "Gauss-Krueger"); *default:* **false** |
| **--proj** {{DT_STR}} | Uses STR as proj.4 definition for projection; *default:* **!** |
| **--proj.inverse** {{DT_BOOL}} | Inverses projection; *default:* **false** |
| **--proj.dhdnutm** {{DT_BOOL}} | Convert from Gauss-Krueger to UTM; *default:* **false** |
| **--proj.plain-geo** {{DT_BOOL}} | Write geo coordinates in output; *default:* **false** |

### Pruning

Sometimes, shapes cover a much larger area than the network. In order to
reduce the amount of data, one can force
**polyconvert** to prune the imported data on the
network's or a given boundary. Read shapes which are completely outside
this boundary are discarded in these cases.

| Option | Description |
|--------|-------------|
| **--prune.in-net** {{DT_BOOL}} | Enables pruning on net boundaries; *default:* **false** |
| **--prune.in-net.offsets** {{DT_STR}} | Uses STR as offset definition added to the net boundaries; *default:* **0,0,0,0** |
| **--prune.boundary** {{DT_STR}} | Uses STR as pruning boundary |
| **--prune.keep-list** {{DT_STR}} | Items in STR will be kept though out of boundary |
| **--prune.explicit** {{DT_STR[]}} | Items with names in STR[] will be removed |

### Processing

| Option | Description |
|--------|-------------|
| **--offset.x** {{DT_FLOAT}} | Adds FLOAT to net x-positions; *default:* **0** |
| **--offset.y** {{DT_FLOAT}} | Adds FLOAT to net y-positions; *default:* **0** |
| **--offset.z** {{DT_FLOAT}} | Adds FLOAT to net z-positions; *default:* **0** |
| **--all-attributes** {{DT_BOOL}} | Imports all attributes as key/value pairs; *default:* **false** |
| **--ignore-errors** {{DT_BOOL}} | Continue on broken input; *default:* **false** |
| **--poi-layer-offset** {{DT_FLOAT}} | Adds FLOAT to the layer value for each poi (i.e. to raise it above polygons); *default:* **0** |

### Building Defaults

When importing shapes for which no type-dependent attributes have been
given, the following default values are used which can be changed on the
command line.

| Option | Description |
|--------|-------------|
| **--color** {{DT_STR}} | Sets STR as default color; *default:* **0.2,0.5,1.** |
| **--prefix** {{DT_STR}} | Sets STR as default prefix |
| **--type** {{DT_STR}} | Sets STR as default type; *default:* **unknown** |
| **--fill** {{DT_BOOL}} | Fills polygons by default; *default:* **true** |
| **--layer** {{DT_FLOAT}} | Sets FLOAT as default layer; *default:* **-1** |
| **--discard** {{DT_BOOL}} | Sets default action to discard; *default:* **false** |

### Report

All applications of the **SUMO**-suite handle most of the reporting
options the same way. These options are discussed at [Basics/Using the
Command Line Applications\#Reporting
Options](Basics/Using_the_Command_Line_Applications.md#reporting_options).

| Option | Description |
|--------|-------------|
| **-v** {{DT_BOOL}}<br> **--verbose** {{DT_BOOL}} | Switches to verbose output; *default:* **false** |
| **--print-options** {{DT_BOOL}} | Prints option values before processing; *default:* **false** |
| **-?** {{DT_BOOL}}<br> **--help** {{DT_BOOL}} | Prints this screen or selected topics; *default:* **false** |
| **-V** {{DT_BOOL}}<br> **--version** {{DT_BOOL}} | Prints the current version; *default:* **false** |
| **-X** {{DT_STR}}<br> **--xml-validation** {{DT_STR}} | Set schema validation scheme of XML inputs ("never", "auto" or "always"); *default:* **auto** |
| **--xml-validation.net** {{DT_STR}} | Set schema validation scheme of SUMO network inputs ("never", "auto" or "always"); *default:* **never** |
| **-W** {{DT_BOOL}}<br> **--no-warnings** {{DT_BOOL}} | Disables output of warnings; *default:* **false** |
| **--aggregate-warnings** {{DT_INT}} | Aggregate warnings of the same type whenever more than INT occur; *default:* **-1** |
| **-l** {{DT_FILE}}<br> **--log** {{DT_FILE}} | Writes all messages to FILE (implies verbose) |
| **--message-log** {{DT_FILE}} | Writes all non-error messages to FILE (implies verbose) |
| **--error-log** {{DT_FILE}} | Writes all warnings and errors to FILE |

# See Also

- [Using additional polygons and POIs within the
  Simulation](Simulation/Shapes.md)
- [Importing shapes from
  OpenStreetMap](Networks/Import/OpenStreetMap.md#importing_additional_polygons_buildings_water_etc)
- [Using configuration
  files](Basics/Using_the_Command_Line_Applications.md#configuration_files)
