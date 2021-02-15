function speedWithoutTraCI = getSpeedWithoutTraCI(vehID)
%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSpeedWithoutTraCI.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
speedWithoutTraCI = traci.vehicle.getUniversal(constants.VAR_SPEED_WITHOUT_TRACI, vehID);