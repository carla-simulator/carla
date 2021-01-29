function setMaxSpeed(vehID, speed)
%setMaxSpeed Sets the maximum speed in m/s for this vehicle.
%   setMaxSpeed(VEHID,SPEED) Sets the maximum speed in m/s for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setMaxSpeed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_MAXSPEED, vehID, speed);