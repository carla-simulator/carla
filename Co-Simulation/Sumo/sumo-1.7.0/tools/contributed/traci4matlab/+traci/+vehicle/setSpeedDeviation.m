function setSpeedDeviation(vehID, deviation)
%setSpeedDeviation Sets the maximum speed deviation for this vehicle.
%   setSpeedDeviation(VEHID,DEVIATION) Sets the maximum speed deviation for 
%   this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setSpeedDeviation.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_SPEED_DEVIATION, vehID, deviation);