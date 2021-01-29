function vehicleClass = getVehicleClass(typeID)
%getVehicleClass Returns the class of vehicles of this type.
%   vehicleClass = getVehicleClass(TYPEID) Returns the class of vehicles of 
%   this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getVehicleClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
vehicleClass = traci.vehicletype.getUniversal(constants.VAR_VEHICLECLASS, typeID);