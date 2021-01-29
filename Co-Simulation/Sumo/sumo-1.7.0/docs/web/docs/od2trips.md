---
title: od2trips
permalink: /od2trips/
---

# From 30.000 feet

**od2trips** imports O/D-matrices and splits them into single vehicle
trips.

- **Purpose:** Conversion of O/D-matrices to single vehicle trips
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):**
  - A) O/D-Matrix
  - B) a set of districts
- **Output:** A list of vehicle trip definitions
- **Programming Language:** C++

# Usage Description

od2trips maps traffic that is defined via origin and destination zones
onto the edges of a network. For details, see
[Demand/Importing_O/D_Matrices](Demand/Importing_O/D_Matrices.md).

## Options

You may use a XML schema definition file for setting up a od2trips
configuration:
[od2tripsConfiguration.xsd](http://sumo.dlr.de/xsd/od2tripsConfiguration.xsd).

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

| Option | Description |
|--------|-------------|
| **-n** {{DT_FILE}}<br> **--taz-files** {{DT_FILE}} | Loads TAZ (districts; also from networks) from FILE(s) |
| **-d** {{DT_FILE}}<br> **--od-matrix-files** {{DT_FILE}} | Loads O/D-files from FILE(s) |
| **--od-amitran-files** {{DT_FILE}} | Loads O/D-matrix in Amitran format from FILE(s) |

### Output

| Option | Description |
|--------|-------------|
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **-o** {{DT_FILE}}<br> **--output-file** {{DT_FILE}} | Writes trip definitions into FILE |
| **--flow-output** {{DT_FILE}} | Writes flow definitions into FILE |
| **--flow-output.probability** {{DT_BOOL}} | Writes probabilistic flow instead of evenly spaced flow; *default:* **false** |
| **--pedestrians** {{DT_BOOL}} | Writes pedestrians instead of vehicles; *default:* **false** |
| **--persontrips** {{DT_BOOL}} | Writes persontrips instead of vehicles; *default:* **false** |
| **--persontrips.modes** {{DT_STR[]}} | Add modes attribute to personTrips |
| **--ignore-vehicle-type** {{DT_BOOL}} | Does not save vtype information; *default:* **false** |

### Time

| Option | Description |
|--------|-------------|
| **-b** {{DT_TIME}}<br> **--begin** {{DT_TIME}} | Defines the begin time; Previous trips will be discarded; *default:* **0** |
| **-e** {{DT_TIME}}<br> **--end** {{DT_TIME}} | Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent; *default:* **9223372036854774** |

### Processing

| Option | Description |
|--------|-------------|
| **-s** {{DT_FLOAT}}<br> **--scale** {{DT_FLOAT}} | Scales the loaded flows by FLOAT; *default:* **1** |
| **--spread.uniform** {{DT_BOOL}} | Spreads trips uniformly over each time period; *default:* **false** |
| **--different-source-sink** {{DT_BOOL}} | Always choose source and sink edge which are not identical; *default:* **false** |
| **--vtype** {{DT_STR}} | Defines the name of the vehicle type to use |
| **--prefix** {{DT_STR}} | Defines the prefix for vehicle names |
| **--timeline** {{DT_STR[]}} | Uses STR[] as a timeline definition |
| **--timeline.day-in-hours** {{DT_BOOL}} | Uses STR as a 24h-timeline definition; *default:* **false** |
| **--no-step-log** {{DT_BOOL}} | Disable console output of current time step; *default:* **false** |

### Defaults

| Option | Description |
|--------|-------------|
| **--departlane** {{DT_STR}} | Assigns a default depart lane; *default:* **free** |
| **--departpos** {{DT_STR}} | Assigns a default depart position |
| **--departspeed** {{DT_STR}} | Assigns a default depart speed; *default:* **max** |
| **--arrivallane** {{DT_STR}} | Assigns a default arrival lane |
| **--arrivalpos** {{DT_STR}} | Assigns a default arrival position |
| **--arrivalspeed** {{DT_STR}} | Assigns a default arrival speed |

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
| **-W** {{DT_BOOL}}<br> **--no-warnings** {{DT_BOOL}} | Disables output of warnings; *default:* **false** |
| **--aggregate-warnings** {{DT_INT}} | Aggregate warnings of the same type whenever more than INT occur; *default:* **-1** |
| **-l** {{DT_FILE}}<br> **--log** {{DT_FILE}} | Writes all messages to FILE (implies verbose) |
| **--message-log** {{DT_FILE}} | Writes all non-error messages to FILE (implies verbose) |
| **--error-log** {{DT_FILE}} | Writes all warnings and errors to FILE |
| **--ignore-errors** {{DT_BOOL}} | Continue on broken input; *default:* **false** |

### Random Number

All applications of the **SUMO**-suite handle randomisation options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Random Number
Options](Basics/Using_the_Command_Line_Applications.md#random_number_options).

| Option | Description |
|--------|-------------|
| **--random** {{DT_BOOL}} | Initialises the random number generator with the current system time; *default:* **false** |
| **--seed** {{DT_INT}} | Initialises the random number generator with the given value; *default:* **23423** |

# Notes

The option --vtype only works when "--od-matrix-files" is used. For
"Amitran" format, this is given by the "id" property of "actorConfig".
