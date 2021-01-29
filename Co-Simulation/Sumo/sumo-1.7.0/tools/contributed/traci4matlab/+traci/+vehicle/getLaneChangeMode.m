function lcMode = getLaneChangeMode(vehID)
%getLaneChangeMode Gets the vehicle's lane change mode as a bitset.
%   lcMode = getLaneChangeMode(VEHID) Gets the vehicle's lane change mode
%   as a bitset.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLaneChangeMode.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
lcMode = traci.vehicle.getUniversal(constants.VAR_LANECHANGE_MODE, vehID);
