function setMaxSpeedLat(typeID, maxSpeedLat)
%setMaxSpeedLat Sets the maximum lateral speed of this type.
%   setMaxSpeedLat(TYPEID,MAXSPEEDLAT) Sets the maximum lateral speed of
%   this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setMaxSpeedLat.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE,...
    constants.VAR_MAXSPEED_LAT, typeID, maxSpeedLat);
