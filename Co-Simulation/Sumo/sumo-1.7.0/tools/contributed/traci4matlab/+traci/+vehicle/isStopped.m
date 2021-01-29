function result = isStopped(vehID)
%isStopped Return whether the vehicle is stopped.
%   isStopped = isStopped(VEHID) Return whether the vehicle is stopped.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: isStopped.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
result = (traci.vehicle.getStopState(vehID) && 1) == 1;
