function width = getWidth(typeID)
%getVehicleClass
%   vehicleClass = getVehicleClass(TYPEID) Returns the width in m of 
%   vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getWidth.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
width = traci.vehicletype.getUniversal(constants.VAR_WIDTH, typeID);