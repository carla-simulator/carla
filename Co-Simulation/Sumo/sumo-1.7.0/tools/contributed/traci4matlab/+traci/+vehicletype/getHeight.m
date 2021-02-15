function height = getHeight(typeID)
%getHeight Vehicle type height.
%   height = getHeight(TYPEID) Returns the height in m of vehicles of this
%   type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getHeight.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
height = traci.vehicletype.getUniversal(constants.VAR_HEIGHT, typeID);
