---
title: Traci/GenericParameters
permalink: /Traci/GenericParameters/
---

# Variable 0x7e: Parameter

[Generic parameters are supported by various simulation
objects](../Simulation/GenericParameters.md). They can be read and
set with the following commands:

## supported domains (get / set)

| domain        | command get | command set |
| ------------- | ----------- | ----------- |
| traffic light | 0xa2        | 0xc2        |
| lane          | 0xa3        | 0xc3        |
| vehicle       | 0xa4        | 0xc4        |
| vehicle type  | 0xa5        | 0xc5        |
| route         | 0xa6        | 0xc6        |
| poi           | 0xa7        | 0xc7        |
| polygon       | 0xa8        | 0xc8        |
| edge          | 0xaa        | 0xca        |
| person        | 0xae        | 0xce        |
| simulation    | 0xab        | 0xcb        |

## get parameter

|          byte          |         byte          |  string   |        byte         |  string   |
| :--------------------: | :-------------------: | :-------: | :-----------------: | :-------: |
| command get for domain | type parameter (0x7e) | object id | value type *string* | parameter |

## set parameter

|          byte    |         byte    |  string   |         byte   | int |        byte         |  string   |        byte         | string |
| :--------------: | :-------------: | :-------: | :------------: | :-: | :-----------------: | :-------: | :-----------------: | :----: |
| command set for domain | type parameter (0x7e) | object id | value type *compound* |  2  | value type *string* | parameter | value type *string* | value  |