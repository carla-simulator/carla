function PMxEmission = getPMxEmission(vehID)
%getPmxEmission Get the particular matter emission of the vehicle.
%   pmxEmission = getPmxEmission(VEHID) Returns the particular matter 
%   emission in mg for the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPMxEmission.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
PMxEmission = traci.vehicle.getUniversal(constants.VAR_PMXEMISSION, vehID);