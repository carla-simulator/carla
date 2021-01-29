function lastStepOccupancy = getLastStepOccupancy(edgeID)
%getLastStepOccupancy Get the percentage of occupation on the edge.
%   lastStepOccupancy = getLastStepOccupancy(EDGEID) Returns the occupancy 
%   in percentage for the last time step on the given edge.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepOccupancy.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepOccupancy = traci.edge.getUniversal(constants.LAST_STEP_OCCUPANCY, edgeID);