function laneIndex = getLaneIndex(vehID)
%getLaneIndex Returns the index of the lane where the vehicle was in the last step.
%   laneIndex = getLaneIndex(VEHID) Returns the index of the lane the named
%   vehicle was at within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLaneIndex.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
laneIndex = traci.vehicle.getUniversal(constants.VAR_LANE_INDEX, vehID);