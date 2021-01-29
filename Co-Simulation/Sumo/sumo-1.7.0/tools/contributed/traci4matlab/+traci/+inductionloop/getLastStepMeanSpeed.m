function lastStepMeanSpeed = getLastStepMeanSpeed(loopID)
%getLastStepMeanSpeed Get the mean speed of the vehicles in the lane.
%   lastStepMeanSpeed = getLastStepMeanSpeed(LOOPID) Returns the mean 
%   speed in m/s of vehicles that were on the named induction loop within 
%   the last simulation step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepMeanSpeed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepMeanSpeed = traci.inductionloop.getUniversal(constants.LAST_STEP_MEAN_SPEED, loopID);