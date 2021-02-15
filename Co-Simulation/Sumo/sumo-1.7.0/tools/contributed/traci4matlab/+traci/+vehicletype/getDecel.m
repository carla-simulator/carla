function decel = getDecel(typeID)
%getDecel Returns the maximum deceleration in of vehicles of this type.
%   decel = getDecel(TYPEID) Returns the maximum deceleration in m/s^2 of 
%   vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getDecel.m 49 2018-12-27 14:08:44Z afacostag $

import traci.constants
decel = traci.vehicletype.getUniversal(constants.VAR_DECEL, typeID);
