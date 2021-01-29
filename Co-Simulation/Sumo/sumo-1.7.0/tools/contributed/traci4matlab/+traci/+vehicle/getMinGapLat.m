function minGapLat = getMinGapLat(vehID)
%getAngle Get the lateral gap at 50km/h.
%   minGapLat = getMinGapLat(VEHID) Returns The desired lateral gap of this
%   vehicle at 50km/h in m.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getMinGapLat.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
minGapLat = traci.vehicle.getUniversal(constants.VAR_MINGAP_LAT, vehID);
