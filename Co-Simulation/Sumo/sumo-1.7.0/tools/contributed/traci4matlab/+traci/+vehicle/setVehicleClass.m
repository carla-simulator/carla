function setVehicleClass(vehID, clazz)
%setVehicleClass Sets the vehicle class for this vehicle.
%   setVehicleClass(VEHID,CLASS) Sets the vehicle class for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setVehicleClass.m 48 2018-12-26 15:35:20Z afacostag $


import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_VEHICLECLASS, vehID, clazz);