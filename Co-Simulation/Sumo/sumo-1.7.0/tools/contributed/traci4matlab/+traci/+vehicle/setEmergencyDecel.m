function setEmergencyDecel(vehID, decel)
%setEmergencyDecel Sets the maximal physical deceleration.
%   setEmergencyDecel(VEHID,DECEL) Sets the maximal physically possible
%   deceleration in m/s^2 for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setEmergencyDecel.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_EMERGENCY_DECEL, vehID, decel);
