function laneID = getLaneID(vehID)
%getLaneID Returns the ID of the lane where the vehicle was in the last step.
%   laneID = getLaneID(VEHID) Returns the id of the lane the named vehicle 
%   was at within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLaneID.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants


import traci.constants
laneID = traci.vehicle.getUniversal(constants.VAR_LANE_ID, vehID);