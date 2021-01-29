function busStopWaiting = getBusStopWaiting(stopID)
%getBusStopWaiting Number of waiting persons at a bus stop.
%   busStopWaiting = getBusStopWaiting() Get the total number of waiting
%   persons at the named bus stop.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getBusStopWaiting.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
busStopWaiting = traci.simulation.getUniversal(constants.VAR_BUS_STOP_WAITING, stopID);
