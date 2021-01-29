function timeSinceDetection = getTimeSinceDetection(loopID)
%getTimeSinceDetection Get the time since the last detection.
%   timeSinceDetection = getTimeSinceDetection(LOOPID) Returns the time in 
%   seconds since last detection.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getTimeSinceDetection.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
timeSinceDetection = traci.inductionloop.getUniversal(constants.LAST_STEP_TIME_SINCE_DETECTION, loopID);