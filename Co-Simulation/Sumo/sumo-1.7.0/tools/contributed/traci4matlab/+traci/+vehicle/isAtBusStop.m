function isIt = isAtBusStop(vehID)
%isAtBusStop Return whether the vehicle is stopped at a bus stop.
%   isIt = isAtBusStop(VEHID) Return whether the vehicle is stopped at a
%   bus stop

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: isAtBusStop.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
isIt = (traci.vehicle.getStopState(vehID) && 16) == 16;
