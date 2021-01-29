function electricityConsumption = getElectricityConsumption(vehID)
%getElectricityConsumption Get the electricity consumption of the vehicle.
%   electricityConsumption = getElectricityConsumption(VEHID) Returns the
%   electricity consumption in mw for the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getElectricityConsumption.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
electricityConsumption = traci.vehicle.getUniversal(constants.VAR_ELECTRICITYCONSUMPTION, vehID);
