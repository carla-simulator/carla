function speedDeviation = getSpeedDeviation(typeID)
%getSpeedDeviation Returns the maximum speed deviation of vehicles of this type.
%   speedDeviation = getSpeedDeviation(TYPEID) Returns the maximum speed 
%   deviation of vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSpeedDeviation.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
speedDeviation = traci.vehicletype.getUniversal(constants.VAR_SPEED_DEVIATION, typeID);