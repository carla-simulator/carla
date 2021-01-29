function maxSpeedLat = getMaxSpeedLat(vehID)
%getMaxSpeedLat Get the maximum lateral speed.
%   maxSpeedLat = getMaxSpeedLat(VEHID) Returns the maximum lateral speed
%   in m/s of this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getMaxSpeedLat.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
maxSpeedLat = traci.vehicle.getUniversal(constants.VAR_MAXSPEED_LAT, vehID);
