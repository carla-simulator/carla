function isParquing = isStoppedParking(vehID)
%isStoppedParking Return whether the vehicle is parquing.
%   isParquing = isStoppedParking(VEHID) Return whether the vehicle is
%   parking (implies stopped)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: isStoppedParking.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
isParquing = (traci.vehicle.getStopState(vehID) && 2) == 2;
