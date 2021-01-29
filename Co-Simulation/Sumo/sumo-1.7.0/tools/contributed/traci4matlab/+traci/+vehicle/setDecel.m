function setDecel(vehID, decel)
%setDecel Sets the maximum deceleration for this vehicle.
%   setDecel(VEHID,DECEL) Sets the maximum deceleration in m/s^2 for this 
%   vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setDecel.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_DECEL, vehID, decel);
