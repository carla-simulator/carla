function CO2Emission = getCO2Emission(vehID)
%getCO2Emission Returns the CO2 emission of the vehicle.
%   CO2Emission = getCO2Emission(VEHID) Returns the CO2 emission in mg for
%   the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getCO2Emission.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
CO2Emission = traci.vehicle.getUniversal(constants.VAR_CO2EMISSION, vehID);