function apparentDecel = getApparentDecel(typeID)
%getApparentDecel Apparent deceleration.
%   apparentDecel = getApparentDecel(TYPEID) Returns the apparent
%   deceleration in m/s^2 of vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getApparentDecel.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
apparentDecel = traci.vehicletype.getUniversal(constants.VAR_APPARENT_DECEL, typeID);