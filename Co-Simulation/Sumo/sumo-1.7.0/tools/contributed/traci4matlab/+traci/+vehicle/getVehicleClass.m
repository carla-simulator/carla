function vehicleClass = getVehicleClass(vehID)
%getVehicleClass Returns the vehicle class of this vehicle.
%   vehicleClass = getVehicleClass(VEHID) Returns the vehicle class of this
%   vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getVehicleClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
vehicleClass = traci.vehicle.getUniversal(constants.VAR_VEHICLECLASS, vehID);