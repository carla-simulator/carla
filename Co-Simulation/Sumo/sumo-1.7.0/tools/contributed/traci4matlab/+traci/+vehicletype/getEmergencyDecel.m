function emergencyDecel = getEmergencyDecel(typeID)
%getEmergencyDecel Maximal physically possible deceleration.
%   emergencyDecel = getEmergencyDecel(TYPEID) Returns the maximal physically
%   possible deceleration in m/s^2 of vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEmergencyDecel.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
emergencyDecel = traci.vehicletype.getUniversal(constants.VAR_EMERGENCY_DECEL, typeID);