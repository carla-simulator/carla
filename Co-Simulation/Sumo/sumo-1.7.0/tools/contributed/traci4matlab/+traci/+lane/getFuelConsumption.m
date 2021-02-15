function FuelConsumption = getFuelConsumption(laneID)
%getFuelConsumption Get the fuel consumption on the lane.
%   fuelConsumption = getFuelConsumption(LANEID) Returns the fuel 
%   consumption in ml for the last time step on the given lane.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getFuelConsumption.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
FuelConsumption = traci.lane.getUniversal(constants.VAR_FUELCONSUMPTION, laneID);