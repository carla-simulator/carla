---
title: Models/Emissions/HBEFA-based
permalink: /Models/Emissions/HBEFA-based/
---

*"The [Handbook Emission Factors for Road Transport
(HBEFA)](http://www.hbefa.net/) provides emission factors for all
current vehicle categories (PC, LDV, HDV and motor cycles), each divided
into different categories, for a wide variety of traffic situations."*
(from <http://www.hbefa.net/>, 30.01.2008). Since version 0.10.2, SUMO
is able to simulate vehicular pollutant emissions based on this database
application. Though HBEFA contains information about other pollutants,
we decided to implement only the following, major ones:

- CO<sub>2</sub>
- CO
- HC
- NO<sub>x</sub>
- PM<sub>x</sub>
- fuel consumption

We have remodeled [HBEFA](http://www.hbefa.net/) data for the following
reasons:

1.  The goal was to have an emission model be directly embedded in SUMO,
    [HBEFA](http://www.hbefa.net/) is a database application for itself
2.  To allow computation of vehicular emissions on a continuous range as
    the original [HBEFA](http://www.hbefa.net/) data cover certain
    speeds only
3.  To avoid having the need to deal with all of
    [HBEFA](http://www.hbefa.net/)'s emission classes

# Vehicle Emission Classes

The original vehicle classes supplied by [HBEFA](http://www.hbefa.net/)
have been clustered. In order to improve the quality, passengers and
light delivery vehicles have been clustered independent to heavy duty
vehicles. For the first set (passenger and light delivery vehicles), two
different sets have been computed, one with 7 classes, one with 14
classes. For the heavy duty vehicles, three sets have been computed,
with respectively 3, 6, and 12 classes. Choosing the proper set is not
easy - on the one hand, more classes mean a better fit of each of these,
on the other hand, the vehicle population mix is more complicated to be
generated.

## Normal Classes

The following tables give the emissionClass strings to use in the
definition of vehicle types.

**Table 1.1 HBEFA heavy duty vehicle types and their clusters in
dependence to the class number/cluster**

| vehClass                   | k = 3     | k = 6     | k = 12      |
| -------------------------- | --------- | --------- | ----------- |
| SMW80er Jahre 12-14t       | HDV_3_2 | HDV_6_6 | HDV_12_8  |
| SMW80er Jahre 14-20t       | HDV_3_2 | HDV_6_6 | HDV_12_8  |
| SMW80er Jahre 20-26t       | HDV_3_2 | HDV_6_4 | HDV_12_4  |
| SMW80er Jahre 28-32t       | HDV_3_2 | HDV_6_4 | HDV_12_4  |
| SMW80er Jahre 28-34t       | HDV_3_2 | HDV_6_4 | HDV_12_4  |
| SMW80er Jahre 7.5-12t      | HDV_3_1 | HDV_6_1 | HDV_12_7  |
| SMW80er Jahre bigger18t    | HDV_3_3 | HDV_6_5 | HDV_12_9  |
| SMW80er Jahre bigger32t    | HDV_3_2 | HDV_6_4 | HDV_12_4  |
| SMW80er Jahre bigger34-40t | HDV_3_2 | HDV_6_4 | HDV_12_4  |
| SMW80er Jahre smaller18t   | HDV_3_2 | HDV_6_4 | HDV_12_12 |
| SMW80er Jahre smaller28t   | HDV_3_2 | HDV_6_6 | HDV_12_8  |
| SMW80er Jahre smaller7.5t  | HDV_3_1 | HDV_6_1 | HDV_12_7  |
| SMWEuro1 12-14t            | HDV_3_1 | HDV_6_1 | HDV_12_3  |
| SMWEuro1 14-20t            | HDV_3_1 | HDV_6_6 | HDV_12_3  |
| SMWEuro1 20-26t            | HDV_3_2 | HDV_6_6 | HDV_12_3  |
| SMWEuro1 28-32t            | HDV_3_2 | HDV_6_6 | HDV_12_6  |
| SMWEuro1 28-34t            | HDV_3_2 | HDV_6_6 | HDV_12_6  |
| SMWEuro1 7.5-12t           | HDV_3_1 | HDV_6_1 | HDV_12_2  |
| SMWEuro1 bigger18t         | HDV_3_3 | HDV_6_3 | HDV_12_11 |
| SMWEuro1 bigger32t         | HDV_3_2 | HDV_6_6 | HDV_12_6  |
| SMWEuro1 bigger34-40t      | HDV_3_2 | HDV_6_6 | HDV_12_6  |
| SMWEuro1 smaller18t        | HDV_3_1 | HDV_6_6 | HDV_12_3  |
| SMWEuro1 smaller28t        | HDV_3_1 | HDV_6_6 | HDV_12_3  |
| SMWEuro1 smaller7.5t       | HDV_3_1 | HDV_6_1 | HDV_12_2  |
| SMWEuro2 12-14t            | HDV_3_1 | HDV_6_1 | HDV_12_1  |
| SMWEuro2 14-20t            | HDV_3_1 | HDV_6_1 | HDV_12_1  |
| SMWEuro2 20-26t            | HDV_3_3 | HDV_6_2 | HDV_12_5  |
| SMWEuro2 28-32t            | HDV_3_3 | HDV_6_2 | HDV_12_5  |
| SMWEuro2 28-34t            | HDV_3_3 | HDV_6_2 | HDV_12_5  |
| SMWEuro2 7.5-12t           | HDV_3_1 | HDV_6_1 | HDV_12_2  |
| SMWEuro2 bigger18t         | HDV_3_3 | HDV_6_3 | HDV_12_11 |
| SMWEuro2 bigger32t         | HDV_3_3 | HDV_6_2 | HDV_12_5  |
| SMWEuro2 bigger34-40t      | HDV_3_3 | HDV_6_2 | HDV_12_5  |
| SMWEuro2 smaller18t        | HDV_3_1 | HDV_6_1 | HDV_12_12 |
| SMWEuro2 smaller28t        | HDV_3_1 | HDV_6_1 | HDV_12_1  |
| SMWEuro2 smaller7.5t       | HDV_3_1 | HDV_6_1 | HDV_12_2  |
| SMWEuro3 12-14t            | HDV_3_1 | HDV_6_1 | HDV_12_1  |
| SMWEuro3 28-32t            | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro3 28-34t            | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro3 7.5-12t           | HDV_3_1 | HDV_6_1 | HDV_12_2  |
| SMWEuro3 bigger18t         | HDV_3_3 | HDV_6_3 | HDV_12_11 |
| SMWEuro3 bigger32t         | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro3 bigger34-40t      | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro3 smaller18t        | HDV_3_1 | HDV_6_1 | HDV_12_12 |
| SMWEuro3 smaller28t        | HDV_3_1 | HDV_6_1 | HDV_12_1  |
| SMWEuro3 smaller7.5t       | HDV_3_1 | HDV_6_1 | HDV_12_2  |
| SMWEuro4 28-34t            | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro4 bigger34-40t      | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro4 smaller18t        | HDV_3_1 | HDV_6_1 | HDV_12_12 |
| SMWEuro5 28-34t            | HDV_3_3 | HDV_6_2 | HDV_12_10 |
| SMWEuro5 smaller18t        | HDV_3_1 | HDV_6_1 | HDV_12_12 |

**Table 1.2 HBEFA passenger and light delivery vehicle types and their
clusters in dependence to the class number/cluster**

| vehClass                      | k = 7   | k = 14    |
| ----------------------------- | ------- | --------- |
| LNFBEuro2 M+N1-I              | P_7_7 | P_14_9  |
| LNFBEuro2 N1-II               | P_7_2 | P_14_1  |
| LNFBEuro3 M+N1-I              | P_7_7 | P_14_9  |
| LNFBEuro4 M+N1-I              | P_7_7 | P_14_9  |
| LNFBGKatsmaller91 M+N1-I      | P_7_3 | P_14_7  |
| LNFBGKatsmaller91 N1-II       | P_7_2 | P_14_3  |
| LNFBKonv M+N1-I               | P_7_4 | P_14_12 |
| LNFBKonv N1-II                | P_7_1 | P_14_2  |
| LNFDEuro1FAV1 M+N1-I          | P_7_5 | P_14_10 |
| LNFDEuro1FAV1 N1-II           | P_7_6 | P_14_6  |
| LNFDEuro2 M+N1-I              | P_7_7 | P_14_8  |
| LNFDEuro2 N1-II               | P_7_6 | P_14_6  |
| LNFDEuro3 M+N1-I              | P_7_7 | P_14_8  |
| LNFDEuro3 N1-II               | P_7_6 | P_14_5  |
| LNFDEuro4 M+N1-I              | P_7_7 | P_14_8  |
| LNFDEuro4 N1-II               | P_7_5 | P_14_5  |
| LNFDkonv M+N1-I               | P_7_5 | P_14_10 |
| LNFDkonv N1-II                | P_7_6 | P_14_6  |
| PKWBEuro1FAV1 1.4-2L          | P_7_3 | P_14_14 |
| PKWBEuro1FAV1 bigger2L        | P_7_3 | P_14_14 |
| PKWBEuro1FAV1 smaller1.4L     | P_7_7 | P_14_14 |
| PKWBEuro2 1.4-2L              | P_7_7 | P_14_9  |
| PKWBEuro2 bigger2L            | P_7_7 | P_14_13 |
| PKWBEuro2 smaller1.4L         | P_7_7 | P_14_9  |
| PKWBEuro3 1.4-2L              | P_7_7 | P_14_9  |
| PKWBEuro3 bigger2L            | P_7_7 | P_14_13 |
| PKWBEuro4 1.4-2L              | P_7_7 | P_14_9  |
| PKWBEuro4 bigger2L            | P_7_7 | P_14_13 |
| PKWBGKatsmaller91 1.4-2L      | P_7_3 | P_14_7  |
| PKWBGKatsmaller91 bigger2L    | P_7_3 | P_14_7  |
| PKWBGKatsmaller91 smaller1.4L | P_7_3 | P_14_7  |
| PKWBKonv 1.4-2L               | P_7_3 | P_14_11 |
| PKWBKonv bigger2L             | P_7_4 | P_14_12 |
| PKWBKonv smaller1.4L          | P_7_3 | P_14_11 |
| PKWDEuro2 1.4-2L              | P_7_7 | P_14_8  |
| PKWDEuro2 bigger2L            | P_7_7 | P_14_4  |
| PKWDEuro3 1.4-2L              | P_7_7 | P_14_8  |
| PKWDEuro3 bigger2L            | P_7_7 | P_14_4  |
| PKWDEuro4 1.4-2L              | P_7_7 | P_14_8  |
| PKWDEuro4 bigger2L            | P_7_7 | P_14_8  |
| PKWDXXIIIFAV1 1.4-2L          | P_7_5 | P_14_10 |
| PKWDXXIIIFAV1 bigger2L        | P_7_5 | P_14_10 |

## Special Classes

The special class `zero` may be used to specify a vehicle without emissions.

In addition to the these strings, it is also possible to add the code `A0`
in order to get emission models which ignore vehicle acceleration.

```
emissionClass="P_A0_7_7"
emissionClass="HDV_A0_3_2"
```