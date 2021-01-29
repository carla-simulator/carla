function emergencyDecel = getEmergencyDecel(vehID)
%getEmergencyDecel Get the emergency deceleration of the vehicle.
%   emergencyDecel = getEmergencyDecel(VEHID) Returns the maximal
%   physically possible deceleration in m/s^2 of this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEmergencyDecel.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
emergencyDecel = traci.vehicle.getUniversal(constants.VAR_EMERGENCY_DECEL, vehID);
