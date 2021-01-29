function setEmergencyDecel(typeID, emergencyDecel)
%setEmergencyDecel Sets the maximal physically possible deceleration.
%   setEmergencyDecel(TYPEID,EMERGENCYDECEL) Sets the maximal physically
%   possible deceleration in m/s^2 of vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setEmergencyDecel.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE,...
    constants.VAR_EMERGENCY_DECEL, typeID, emergencyDecel);
