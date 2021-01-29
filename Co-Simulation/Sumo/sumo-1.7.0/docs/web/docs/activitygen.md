---
title: activitygen
permalink: /activitygen/
---

# From 30.000 feet

**activitygen** reads the definition of a
population matching an also given network. It computes and mobility
wishes for this population.

- **Purpose:** Demand generation for a synthetic population
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):**

  A) a road network as generated via
  [netconvert](netconvert.md) or
  [netgenerate](netgenerate.md), see [Building Networks](index.md#network_building)

  B) a population definition, see [Activity-based Demand Generation](Demand/Activity-based_Demand_Generation.md)

- **Output:** [Definition of Vehicles, Vehicle Types, and
  Routes](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
  usable by [sumo](sumo.md)
- **Programming Language:** C++

# Usage Description

A step by step description for using
**activitygen** can be found [here](Demand/Activity-based_Demand_Generation.md)

## Options

You may use a XML schema definition file for setting up a activitygen
configuration:
[activitygenConfiguration.xsd](http://sumo.dlr.de/xsd/activitygenConfiguration.xsd).

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
| **-n** {{DT_FILE}}<br> **--net-file** {{DT_FILE}} | Use FILE as SUMO-network to create trips for |
| **-s** {{DT_FILE}}<br> **--stat-file** {{DT_FILE}} | Loads the SUMO-statistics FILE |

### Output

| Option | Description |
|--------|-------------|
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **-o** {{DT_FILE}}<br> **--output-file** {{DT_FILE}} | Write generated trips to FILE |

### Time

| Option | Description |
|--------|-------------|
| **-b** {{DT_INT}}<br> **--begin** {{DT_INT}} | Sets the time of beginning of the simulation during the first day (in seconds); *default:* **0** |
| **-e** {{DT_INT}}<br> **--end** {{DT_INT}} | Sets the time of ending of the simulation during the last day (in seconds); *default:* **0** |
| **--duration-d** {{DT_INT}} | Sets the duration of the simulation in days; *default:* **1** |

### Report

All applications of the **SUMO**-suite handle most of the reporting
options the same way. These options are discussed at [Basics/Using the
Command Line Applications\#Reporting Options](Basics/Using_the_Command_Line_Applications.md#reporting_options).

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
| **--debug** {{DT_BOOL}} | Detailed messages about every single step; *default:* **false** |

### Random Number

All applications of the **SUMO**-suite handle randomisation options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Random Number Options](Basics/Using_the_Command_Line_Applications.md#random_number_options).

| Option | Description |
|--------|-------------|
| **--random** {{DT_BOOL}} | Initialises the random number generator with the current system time; *default:* **false** |
| **--seed** {{DT_INT}} | Initialises the random number generator with the given value; *default:* **23423** |


