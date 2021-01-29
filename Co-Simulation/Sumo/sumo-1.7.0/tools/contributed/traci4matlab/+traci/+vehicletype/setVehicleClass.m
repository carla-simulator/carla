function setVehicleClass(typeID, clazz)
%setVehicleClass Sets the class for vehicles of this type.
%   setVehicleClass(TYPEID,CLASS) Sets the class for vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setVehicleClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_VEHICLECLASS, typeID, clazz);