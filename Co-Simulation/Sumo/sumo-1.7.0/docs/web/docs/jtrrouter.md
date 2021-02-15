---
title: jtrrouter
permalink: /jtrrouter/
---

# From 30.000 feet

**jtrrouter** computes routes that may be used by
[sumo](sumo.md) based on traffic volumes and junction turning
ratios.

- **Purpose:** Building vehicle routes from demand definitions using
junction turning percentages
- **System:** portable (Linux/Windows is tested); runs on command line
  - **Input (mandatory):**

    A) a road network as generated via
    [netconvert](netconvert.md) or
    [netgenerate](netgenerate.md), see [Building
    Networks](index.md#network_building)

    B) a demand definition, see [Demand
    Modelling](index.md#demand_modelling)

    C) Junction turning definitions

- **Output:** [Definition of Vehicles, Vehicle Types, and
Routes](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
usable by [sumo](sumo.md)
- **Programming Language:** C++

# Usage Description

jtrrouter is made for [routing based on
turn-ratios](Demand/Routing_by_Turn_Probabilities.md).

## Options

You may use a XML schema definition file for setting up a jtrrouter
configuration:
[jtrrouterConfiguration.xsd](http://sumo.dlr.de/xsd/jtrrouterConfiguration.xsd).

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
| **-n** {{DT_FILE}}<br> **--net-file** {{DT_FILE}} | Use FILE as SUMO-network to route on |
| **-d** {{DT_FILE}}<br> **--additional-files** {{DT_FILE}} | Read additional network data (districts, bus stops) from FILE(s) |
| **-r** {{DT_FILE}}<br> **--route-files** {{DT_FILE}} | Read sumo routes, alternatives, flows, and trips from FILE(s) |
| **--phemlight-path** {{DT_FILE}} | Determines where to load PHEMlight definitions from.; *default:* **./PHEMlight/** |
| **--junction-taz** {{DT_BOOL}} | Initialize a TAZ for every junction to use attributes toJunction and fromJunction; *default:* **false** |
| **-t** {{DT_FILE}}<br> **--turn-ratio-files** {{DT_FILE}} | Read turning ratios from FILE(s) |

### Output

| Option | Description |
|--------|-------------|
| **-o** {{DT_FILE}}<br> **--output-file** {{DT_FILE}} | Write generated routes to FILE |
| **--vtype-output** {{DT_FILE}} | Write used vehicle types into separate FILE |
| **--keep-vtype-distributions** {{DT_BOOL}} | Keep vTypeDistribution ids when writing vehicles and their types; *default:* **false** |
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **--exit-times** {{DT_BOOL}} | Write exit times (weights) for each edge; *default:* **false** |

### Processing

| Option | Description |
|--------|-------------|
| **--unsorted-input** {{DT_BOOL}} | Assume input is unsorted; *default:* **false** |
| **-s** {{DT_TIME}}<br> **--route-steps** {{DT_TIME}} | Load routes for the next number of seconds ahead; *default:* **200** |
| **--no-internal-links** {{DT_BOOL}} | Disable (junction) internal links; *default:* **false** |
| **--randomize-flows** {{DT_BOOL}} | generate random departure times for flow input; *default:* **false** |
| **--max-alternatives** {{DT_INT}} | Prune the number of alternatives to INT; *default:* **5** |
| **--remove-loops** {{DT_BOOL}} | Remove loops within the route; Remove turnarounds at start and end of the route; *default:* **false** |
| **--repair** {{DT_BOOL}} | Tries to correct a false route; *default:* **false** |
| **--repair.from** {{DT_BOOL}} | Tries to correct an invalid starting edge by using the first usable edge instead; *default:* **false** |
| **--repair.to** {{DT_BOOL}} | Tries to correct an invalid destination edge by using the last usable edge instead; *default:* **false** |
| **--weights.interpolate** {{DT_BOOL}} | Interpolate edge weights at interval boundaries; *default:* **false** |
| **--weights.minor-penalty** {{DT_FLOAT}} | Apply the given time penalty when computing routing costs for minor-link internal lanes; *default:* **1.5** |
| **--with-taz** {{DT_BOOL}} | Use origin and destination zones (districts) for in- and output; *default:* **false** |
| **--mapmatch.distance** {{DT_FLOAT}} | Maximum distance when mapping input coordinates (fromXY etc.) to the road network; *default:* **100** |
| **--mapmatch.junctions** {{DT_BOOL}} | Match positions to junctions instead of edges; *default:* **false** |
| **--bulk-routing** {{DT_BOOL}} | Aggregate routing queries with the same origin; *default:* **false** |
| **--routing-threads** {{DT_INT}} | The number of parallel execution threads used for routing; *default:* **0** |
| **--restriction-params** {{DT_STR[]}} | Comma separated list of param keys to compare for additional restrictions |
| **--max-edges-factor** {{DT_FLOAT}} | Routes are cut off when the route edges to net edges ratio is larger than FLOAT; *default:* **2** |
| **-T** {{DT_STR[]}}<br> **--turn-defaults** {{DT_STR[]}} | Use STR[] as default turn definition; *default:* **30,50,20** |
| **--sink-edges** {{DT_STR[]}} | Use STR[] as list of sink edges |
| **-A** {{DT_BOOL}}<br> **--accept-all-destinations** {{DT_BOOL}} | Whether all edges are allowed as sink edges; *default:* **false** |
| **-i** {{DT_BOOL}}<br> **--ignore-vclasses** {{DT_BOOL}} | Ignore road restrictions based on vehicle class; *default:* **false** |
| **--allow-loops** {{DT_BOOL}} | Allow to re-use a road; *default:* **false** |
| **-S** {{DT_BOOL}}<br> **--sources-are-sinks** {{DT_BOOL}} | Use all source edges as sink edges.; *default:* **false** |
| **-D** {{DT_BOOL}}<br> **--discount-sources** {{DT_BOOL}} | Subtract upstream flow when inserting a new flow. When option --sources-are-sinks is set, the upstream flow is limited to the value of the source flow and the remaining part terminates.; *default:* **false** |

### Defaults

| Option | Description |
|--------|-------------|
| **--departlane** {{DT_STR}} | Assigns a default depart lane |
| **--departpos** {{DT_STR}} | Assigns a default depart position |
| **--departspeed** {{DT_STR}} | Assigns a default depart speed |
| **--arrivallane** {{DT_STR}} | Assigns a default arrival lane |
| **--arrivalpos** {{DT_STR}} | Assigns a default arrival position |
| **--arrivalspeed** {{DT_STR}} | Assigns a default arrival speed |
| **--defaults-override** {{DT_BOOL}} | Defaults will override given values; *default:* **false** |

### Time

| Option | Description |
|--------|-------------|
| **-b** {{DT_TIME}}<br> **--begin** {{DT_TIME}} | Defines the begin time; Previous trips will be discarded; *default:* **0** |
| **-e** {{DT_TIME}}<br> **--end** {{DT_TIME}} | Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent; *default:* **9223372036854774** |

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
| **--xml-validation.routes** {{DT_STR}} | Set schema validation scheme of SUMO route inputs ("never", "auto" or "always"); *default:* **auto** |
| **-W** {{DT_BOOL}}<br> **--no-warnings** {{DT_BOOL}} | Disables output of warnings; *default:* **false** |
| **--aggregate-warnings** {{DT_INT}} | Aggregate warnings of the same type whenever more than INT occur; *default:* **-1** |
| **-l** {{DT_FILE}}<br> **--log** {{DT_FILE}} | Writes all messages to FILE (implies verbose) |
| **--message-log** {{DT_FILE}} | Writes all non-error messages to FILE (implies verbose) |
| **--error-log** {{DT_FILE}} | Writes all warnings and errors to FILE |
| **--ignore-errors** {{DT_BOOL}} | Continue if a route could not be build; *default:* **false** |
| **--stats-period** {{DT_INT}} | Defines how often statistics shall be printed; *default:* **-1** |
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


