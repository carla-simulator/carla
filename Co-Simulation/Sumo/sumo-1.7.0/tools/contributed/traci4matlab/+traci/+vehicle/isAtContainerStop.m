function isIt = isAtContainerStop(vehID)
%isAtContainerStop Return whether the vehicle is stopped at a container
%stop.
%   isIt = isAtContainerStop(VEHID) Return whether the vehicle is stopped
%   at a container stop.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: isAtContainerStop.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
isIt = (traci.vehicle.getStopState(vehID) && 32) == 32;
