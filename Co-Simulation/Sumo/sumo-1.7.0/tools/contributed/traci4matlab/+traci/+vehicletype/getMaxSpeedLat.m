function maxSpeedLat = getMaxSpeedLat(typeID)
%getMaxSpeedLat Maximum lateral speed.
%   maxSpeedLat = getMaxSpeedLat(TYPEID) Returns the maximum lateral speed
%   in m/s of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getMaxSpeedLat.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
maxSpeedLat = traci.vehicletype.getUniversal(constants.VAR_MAXSPEED_LAT, typeID);
