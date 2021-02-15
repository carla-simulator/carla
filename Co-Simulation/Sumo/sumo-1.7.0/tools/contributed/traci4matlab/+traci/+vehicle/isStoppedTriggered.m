function isTriggered = isStoppedTriggered(vehID)
%isStoppedTriggered Return whether the vehicle is stopped (triggered).
%   isTriggered = isStoppedTriggered(VEHID) Return whether the vehicle is
%   stopped and waiting for a person or container.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: isStoppedTriggered.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
isTriggered = (traci.vehicle.getStopState(vehID) && 12) > 0;
