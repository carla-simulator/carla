---
title: Models/Emissions/HBEFA3-based
permalink: /Models/Emissions/HBEFA3-based/
---

Since version 0.21.0, SUMO is able to simulate vehicular pollutant
emissions based on the database application HBEFA in version 3.1. Though
HBEFA contains information about other pollutants, we decided to
implement only the following, major ones:

- CO<sub>2</sub>
- CO
- HC
- NO<sub>x</sub>
- PM<sub>x</sub>
- fuel consumption

## Vehicle Emission Classes

The vehicle classes below map directly to vehicle classes supplied by
[HBEFA](http://www.hbefa.net/). This is a major difference to the
clustering done with [HBEFA
2.1](../../Models/Emissions/HBEFA-based.md). Note, that within HBEFA
these classes are aggregates of more specific vehicle classes. To use
the emission classes below prepend the class name with the HBEFA3
prefix, e.g. `emissionClass="HBEFA3/PC_G_EU4"`.

The following tables give the emissionClass strings to use in the
definition of vehicle types.

### HBEFA3 heavy duty emission classes

| emissionClass | description                                  |
| ------------- | -------------------------------------------- |
| Bus           | average urban bus (all fuel types)           |
| Coach         | average long distance bus (all fuel types)   |
| HDV           | average heavy duty vehicle (all fuel types)  |
| HDV_G        | average gasoline driven heavy duty vehicle   |
| HDV_D_EU0   | diesel driven heavy duty vehicle Euro norm 0 |
| HDV_D_EU1   | diesel driven heavy duty vehicle Euro norm 1 |
| HDV_D_EU2   | diesel driven heavy duty vehicle Euro norm 2 |
| HDV_D_EU3   | diesel driven heavy duty vehicle Euro norm 3 |
| HDV_D_EU4   | diesel driven heavy duty vehicle Euro norm 4 |
| HDV_D_EU5   | diesel driven heavy duty vehicle Euro norm 5 |
| HDV_D_EU6   | diesel driven heavy duty vehicle Euro norm 6 |
| HDV_D_East  |                                              |

### HBEFA3 passenger and light delivery emission classes

| emissionClass   | description                                    |
| --------------- | ---------------------------------------------- |
| zero            | zero emission vehicle                          |
| LDV             | average light duty vehicles (all fuel types)   |
| LDV_G_EU0     | gasoline driven light duty vehicle Euro norm 0 |
| LDV_G_EU1     | gasoline driven light duty vehicle Euro norm 1 |
| LDV_G_EU2     | gasoline driven light duty vehicle Euro norm 2 |
| LDV_G_EU3     | gasoline driven light duty vehicle Euro norm 3 |
| LDV_G_EU4     | gasoline driven light duty vehicle Euro norm 4 |
| LDV_G_EU5     | gasoline driven light duty vehicle Euro norm 5 |
| LDV_G_EU6     | gasoline driven light duty vehicle Euro norm 6 |
| LDV_G_East    |                                                |
| LDV_D_EU0     | diesel driven light duty vehicle Euro norm 0   |
| LDV_D_EU1     | diesel driven light duty vehicle Euro norm 1   |
| LDV_D_EU2     | diesel driven light duty vehicle Euro norm 2   |
| LDV_D_EU3     | diesel driven light duty vehicle Euro norm 3   |
| LDV_D_EU4     | diesel driven light duty vehicle Euro norm 4   |
| LDV_D_EU5     | diesel driven light duty vehicle Euro norm 5   |
| LDV_D_EU6     | diesel driven light duty vehicle Euro norm 6   |
| PC              | average passenger car (all fuel types)         |
| PC_Alternative | passenger car with alternative fuel technology |
| PC_G_EU0      | gasoline driven passenger car Euro norm 0      |
| PC_G_EU1      | gasoline driven passenger car Euro norm 1      |
| PC_G_EU2      | gasoline driven passenger car Euro norm 2      |
| PC_G_EU3      | gasoline driven passenger car Euro norm 3      |
| PC_G_EU4      | gasoline driven passenger car Euro norm 4      |
| PC_G_EU5      | gasoline driven passenger car Euro norm 5      |
| PC_G_EU6      | gasoline driven passenger car Euro norm 6      |
| PC_G_East     |                                                |
| PC_D_EU0      | diesel driven passenger car Euro norm 0        |
| PC_D_EU1      | diesel driven passenger car Euro norm 1        |
| PC_D_EU2      | diesel driven passenger car Euro norm 2        |
| PC_D_EU3      | diesel driven passenger car Euro norm 3        |
| PC_D_EU4      | diesel driven passenger car Euro norm 4        |
| PC_D_EU5      | diesel driven passenger car Euro norm 5        |
| PC_D_EU6      | diesel driven passenger car Euro norm 6        |

Not all combinations of emissionClass and pollutant give meaningful
results. The following combinations are known to be faulty as of SUMO
0.21.0: LDV + NOx, LDV_G_EU0 + NOx, LDV_G_East + NOx, LDV_G_East +
PMx, PC_G_EU0 + NOx