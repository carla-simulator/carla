---
title: TraCI/Calibrator
permalink: /TraCI/Calibrator/
---
Asks for the value of a certain variable of the named Calibrator.
The value returned is the state of the asked variable/value within the
last simulation step. Please note that for asking values from your
Calibrator [you have to define
them](../Simulation/Calibrator.md)
within an {{AdditionalFile}} and load them at the start of the simulation. The `freq` and `file`
attributes do not affect TraCI.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Calibrator Variables**
</center>

| Variable                                     | ValueType           | Description       |  [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)    |
| -------------------------------------------- | ------------------- | ----------------- | -------------------------------------------------------------- |
| begin                                  | double          | Returns the begin time of the current calibration interval  | [getBegin](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getBegin) |    
| edge ID                                  | string          | Returns the edge of this calibrator  | [getEdgeID](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getEdgeID) |    
| end                                  | double          | Returns the end time of the current calibration interval  | [getEnd](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getEnd) |    
| insert                                  | double          | Returns the number of inserted vehicles in the current calibration interval  | [getInserted](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getInserted) |    
| lane ID                                  | string          | Returns the lane of this calibrator (if it applies to a single lane)| [getLaneID](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getLaneID) |    
| passed                                  | double          | Returns the number of passed vehicles in the current calibration interval  | [getPassed](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getPassed) |    
| removed                                  | double          | Returns the number of removed vehicles in the current calibration interval  | [getRemoved](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getRemoved) |    
| route ID                                  | string          | Returns the route id for the current calibration interval  | [getRouteID](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getRouteID) |    
| routeProbe ID                                 | double          | Returns the routeProbe id for this calibrator  | [getRouteProbeID](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getRouteProbeID) |    
| speed                                  | double          | Returns the target speed of the current calibration interval  | [getSpeed](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getSpeed) |    
| type ID                                  | string          | Returns the type id for the current calibration interval  | [getTypeID](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getTypeID) |    
| vTypes                                  | string          | Returns a list of all types to which the calibrator applies (in a type filter is active)  | [getVTypes](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getVTypes) |    
| vehicle per hour                                  | double          | Returns the number of vehicles per hour in the current calibration interval  | [getVehsPerHour](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-getVehsPerHour) |    
| flow                                  | compound (string, double, double, double, double, string, string, string, string), see below          | Update or add a calibrator interval  | [setFlow](https://sumo.dlr.de/pydoc/traci._calibrator.html#CalibratorDomain-setFlow) |    

# compound message details

The message contents are as following:

### flow

|       string        |    double     |     double     | double  |     string      |    string    |       string     |   string  |  
| :-------------: | :-------------: | :-------------: | :-----: | :--------------: | :----------: | :------------: | :--------: | 
| calibratorID | begin | end | vehsPerHour | speed | typeID | routeID | departLane='first' | departSpeed='max' |
