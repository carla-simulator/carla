function apparentDecel = getApparentDecel(vehID)
%getApparentDecel Get the aparent deceleration of the vehicle.
%   apparentDecel = getApparentDecel(VEHID) Returns the apparent
%   deceleration in m/s^2 of this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getApparentDecel.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
apparentDecel = traci.vehicle.getUniversal(constants.VAR_APPARENT_DECEL, vehID);
