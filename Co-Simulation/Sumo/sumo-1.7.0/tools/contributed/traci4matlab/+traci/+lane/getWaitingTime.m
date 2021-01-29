function waitingTime = getWaitingTime(laneID)
%getWaitingTime Returns the waiting time in seconds.
%   waitingTime = getWaitingTime(LANEID) eturns the waiting time in seconds
%   for the given lane.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getWaitingTime.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

waitingTime = traci.lane.getUniversal(constants.VAR_WAITING_TIME, laneID);