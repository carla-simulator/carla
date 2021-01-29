function lastStepMeanSpeed = getLastStepMeanSpeed(laneID)
%getLastStepMeanSpeed Get the average speed on the lane.
%   lastStepMeanSpeed = getLastStepMeanSpeed(LANEID) Returns the average 
%   speed in m/s for the last time step on the given lane.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepMeanSpeed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepMeanSpeed = traci.lane.getUniversal(constants.LAST_STEP_MEAN_SPEED, laneID);