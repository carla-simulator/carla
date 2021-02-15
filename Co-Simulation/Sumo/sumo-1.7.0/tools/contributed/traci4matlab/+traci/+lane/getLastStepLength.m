function lastStepLength = getLastStepLength(laneID)
%getLastStepLength Get the mean vehicle length on the lane.
%   lastStepLength = getLastStepLength(LANEID) Returns the mean vehicle 
%   length in m for the last time step on the given lane.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepLength.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepLength = traci.lane.getUniversal(constants.LAST_STEP_LENGTH, laneID);