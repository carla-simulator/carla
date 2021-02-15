function bestLanes = getBestLanes(vehID)
%getBestLanes Returns information about the wish to use subsequent lanes
%   bestLanes = getBestLanes(VEHID) Returns information about the wish to 
%   use subsequent edges' lanes, which is stored in a cell array. The
%   information includes laneID, length, occupation, offset, 
%   allowsContinuation and nextLanes. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getBestLanes.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
bestLanes = traci.vehicle.getUniversal(constants.VAR_BEST_LANES, vehID);