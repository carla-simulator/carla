function electricityConsumption = getElectricityConsumption(edgeID)
%getElectricityConsumption Returns the electricity consumption on the given edge.
%   electricityConsumption = getElectricityConsumption(EDGEID) Returns the
%   electricity consumption in mw for the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getElectricityConsumption.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
electricityConsumption = traci.edge.getUniversal(constants.VAR_ELECTRICITYCONSUMPTION, edgeID);
