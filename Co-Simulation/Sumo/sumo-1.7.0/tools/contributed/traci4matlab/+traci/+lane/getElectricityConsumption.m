function electricityConsupmtion = getElectricityConsumption(laneID)
%getElectricityConsumption Returns the electricity consumption in mW for
%   the last time step.
%   electricityConsupmtion = getElectricityConsumption(LANEID) Returns
%   the electricity consumption in mW of the given lane for the last time
%   step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getElectricityConsumption.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

electricityConsupmtion = traci.lane.getUniversal(constants.VAR_ELECTRICITYCONSUMPTION, laneID);