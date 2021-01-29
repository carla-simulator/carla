---
title: dfrouter
permalink: /dfrouter/
---

# From 30.000 feet

**dfrouter** uses induction loop values to compute vehicle routes that
may be used by [sumo](sumo.md).

- **Purpose:** Building vehicle routes from induction loop counts
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):**

  A) a road network as generated via [netconvert](netconvert.md) or [netgenerate](netgenerate.md), see [Building Networks](index.md#network_building)
  
  B) induction loop definitions

  C) induction loop measures

- **Output:** [Definition of Vehicles, Vehicle Types, and
  Routes](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
  usable by [sumo](sumo.md)
- **Programming Language:** C++

# Usage Description

A high level description can be found at
[Demand/Routes_from_Observation_Points](Demand/Routes_from_Observation_Points.md).
The complete list of options is given in the following.

## Options

You may use a XML schema definition file for setting up a dfrouter
configuration:
[dfrouterConfiguration.xsd](http://sumo.dlr.de/xsd/dfrouterConfiguration.xsd).

### Configuration

All applications of the **SUMO**-suite handle configuration options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Configuration Files](Basics/Using_the_Command_Line_Applications.md#configuration_files).

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
| **-n** {{DT_FILE}}<br> **--net-file** {{DT_FILE}} | Loads the SUMO-network FILE |
| **-d** {{DT_FILE}}<br> **--detector-files** {{DT_FILE}} | Loads detector descriptions from FILE |
| **-f** {{DT_FILE}}<br> **--measure-files** {{DT_FILE}} | Loads detector flows from FILE(s) |

### Output

| Option | Description |
|--------|-------------|
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **-o** {{DT_FILE}}<br> **--routes-output** {{DT_FILE}} | Saves computed routes to FILE |
| **--routes-for-all** {{DT_BOOL}} | Forces dfrouter to compute routes for in-between detectors; *default:* **false** |
| **--detector-output** {{DT_FILE}} | Saves typed detectors to FILE |
| **--detectors-poi-output** {{DT_FILE}} | Saves detector positions as pois to FILE |
| **--emitters-output** {{DT_FILE}} | Saves emitter definitions for source detectors to FILE |
| **--vtype** {{DT_BOOL}} | Add vehicle types to the emitters file (PKW, LKW); *default:* **false** |
| **--vtype-output** {{DT_FILE}} | Write generated vehicle types into separate FILE instead of including them into the emitters-output |
| **--emitters-poi-output** {{DT_FILE}} | Saves emitter positions as pois to FILE |
| **--variable-speed-sign-output** {{DT_FILE}} | Saves variable seed sign definitions for sink detectors to FILE |
| **--end-reroute-output** {{DT_FILE}} | Saves rerouter definitions for sink detectors to FILE |
| **--validation-output** {{DT_FILE}} |  |
| **--validation-output.add-sources** {{DT_BOOL}} | *default:* **false** |

### Processing

| Option | Description |
|--------|-------------|
| **--guess-empty-flows** {{DT_BOOL}} | Derive missing flow values from upstream or downstream (not working!); *default:* **false** |
| **-h** {{DT_BOOL}}<br> **--highway-mode** {{DT_BOOL}} | Switches to highway-mode; *default:* **false** |
| **--ignore-invalid-detectors** {{DT_BOOL}} | Only warn about unparseable detectors; *default:* **false** |
| **--revalidate-detectors** {{DT_BOOL}} | Recomputes detector types even if given; *default:* **false** |
| **--revalidate-routes** {{DT_BOOL}} | Recomputes routes even if given; *default:* **false** |
| **--keep-unfinished-routes** {{DT_BOOL}} | Keeps routes even if they have exhausted max-search-depth; *default:* **false** |
| **--keep-longer-routes** {{DT_BOOL}} | Keeps routes even if a shorter one exists; *default:* **false** |
| **--max-search-depth** {{DT_INT}} | Number of edges to follow a route without passing a detector; *default:* **30** |
| **--emissions-only** {{DT_BOOL}} | Writes only emission times; *default:* **false** |
| **--disallowed-edges** {{DT_STR[]}} | Do not route on these edges |
| **--keep-turnarounds** {{DT_BOOL}} | Allow turnarounds as route continuations; *default:* **false** |
| **--min-route-length** {{DT_FLOAT}} | Minimum distance in meters between start and end node of every route; *default:* **-1** |
| **--randomize-flows** {{DT_BOOL}} | generate random departure times for emitted vehicles; *default:* **false** |
| **--time-factor** {{DT_TIME}} | Multiply flow times with TIME to get seconds; *default:* **60** |
| **--time-offset** {{DT_TIME}} | Subtracts TIME seconds from (scaled) flow times; *default:* **0** |
| **--time-step** {{DT_TIME}} | Expected distance between two successive data sets; *default:* **60** |
| **--calibrator-output** {{DT_BOOL}} | Write calibrators to FILE; *default:* **false** |
| **--include-unused-routes** {{DT_BOOL}} | *default:* **false** |
| **--revalidate-flows** {{DT_BOOL}} | *default:* **false** |
| **--remove-empty-detectors** {{DT_BOOL}} | Removes empty detectors from the list; *default:* **false** |
| **--strict-sources** {{DT_BOOL}} | *default:* **false** |
| **--respect-concurrent-inflows** {{DT_BOOL}} | Try to determine further inflows to an inbetween detector when computing split probabilities; *default:* **false** |
| **--scale** {{DT_FLOAT}} | Scale factor for flows; *default:* **1** |

### Defaults

| Option | Description |
|--------|-------------|
| **--departlane** {{DT_STR}} | Assigns a default depart lane |
| **--departpos** {{DT_STR}} | Assigns a default depart position |
| **--departspeed** {{DT_STR}} | Assigns a default depart speed |
| **--arrivallane** {{DT_STR}} | Assigns a default arrival lane |
| **--arrivalpos** {{DT_STR}} | Assigns a default arrival position |
| **--arrivalspeed** {{DT_STR}} | Assigns a default arrival speed |
| **--speeddev** {{DT_FLOAT}} | The default speed deviation of vehicles; *default:* **0.1** |

### Time

| Option | Description |
|--------|-------------|
| **-b** {{DT_TIME}}<br> **--begin** {{DT_TIME}} | Defines the begin time; Previous defs will be discarded; *default:* **0** |
| **-e** {{DT_TIME}}<br> **--end** {{DT_TIME}} | Defines the end time; Later defs will be discarded; Defaults to one day; *default:* **86400** |

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
| **--report-empty-detectors** {{DT_BOOL}} | Lists detectors with no flow (enable -v); *default:* **false** |
| **--print-absolute-flows** {{DT_BOOL}} | Prints aggregated detector flows; *default:* **false** |
| **--no-step-log** {{DT_BOOL}} | Disable console output of route parsing step; *default:* **false** |

### Random Number

All applications of the **SUMO**-suite handle randomisation options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Random Number
Options](Basics/Using_the_Command_Line_Applications.md#random_number_options).

| Option | Description |
|--------|-------------|
| **--random** {{DT_BOOL}} | Initialises the random number generator with the current system time; *default:* **false** |
| **--seed** {{DT_INT}} | Initialises the random number generator with the given value; *default:* **23423** |


