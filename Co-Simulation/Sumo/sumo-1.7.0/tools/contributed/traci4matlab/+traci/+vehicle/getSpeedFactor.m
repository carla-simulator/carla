function speedFactor = getSpeedFactor(vehID)
%getSpeedFactor Returns the chosen speed factor for this vehicle.
%   speedFactor = getSpeedFactor(VEHID) Returns the chosen speed factor for
%   this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSpeedFactor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
speedFactor = traci.vehicle.getUniversal(constants.VAR_SPEED_FACTOR, vehID);