function speedMode = getSpeedMode(vehID)
%getSpeedMode Get the speed mode of the vehicle.
%   speedMode = getSpeedMode(VEHID) Returns the speed mode of the vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSpeedMode.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
speedMode = traci.vehicle.getUniversal(constants.VAR_SPEEDSETMODE, vehID);
