function stepLength = getActionStepLength(vehID)
%getActionStepLength Returns the action step length for this vehicle.
%   stepLength = getActionStepLength(VEHID) Returns the action step length
%   for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getActionStepLength.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
stepLength = traci.vehicle.getUniversal(constants.VAR_ACTIONSTEPLENGTH, vehID);
