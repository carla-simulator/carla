function tau = getTau(typeID)
%getTau Returns the driver's reaction time in s for vehicles of this type.
%   tau = getTau(TYPEID) Returns the driver's reaction time in s for 
%   vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getTau.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
tau = traci.vehicletype.getUniversal(constants.VAR_TAU, typeID);