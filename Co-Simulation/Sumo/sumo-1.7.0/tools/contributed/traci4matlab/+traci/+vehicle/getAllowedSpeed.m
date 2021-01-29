function allowedSpeed = getAllowedSpeed(vehID)
%getAngle Get the maximum allowed speed.
%   allowedSpeed = getAllowedSpeed(VEHID) Returns the maximum allowed speed
%   on the current lane regarding speed factor in m/s for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getAllowedSpeed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
allowedSpeed = traci.vehicle.getUniversal(constants.VAR_ALLOWED_SPEED, vehID);
