function setMinGapLat(vehID, minGapLat)
%setMinGapLat Sets the minimum lateral gap.
%   setMinGapLat(VEHID,MINGAPLAT) Sets the minimum lateral gap of the
%   vehicle at 50km/h in m.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setMinGapLat.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_MINGAP_LAT, vehID, minGapLat);
