function fuelConsumption = getFuelConsumption(vehID)
%getFuelConsumption Get the fuel consumption of the vehicle.
%   fuelConsumption = getFuelConsumption(VEHID) Returns the fuel 
%   consumption in ml for the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getFuelConsumption.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
fuelConsumption = traci.vehicle.getUniversal(constants.VAR_FUELCONSUMPTION, vehID);