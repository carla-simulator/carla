function lastStepVehicleIDs = getLastStepVehicleIDs(loopID)
%getLastStepVehicleIDs Get the IDs of the vehicles that were in the loop.
%   lastStepVehicleIDs = getLastStepVehicleIDs(LOOPID) Returns the list of 
%   ids of vehicles that were on the named induction loop in the last 
%   simulation step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepVehicleIDs.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepVehicleIDs = traci.inductionloop.getUniversal(constants.LAST_STEP_VEHICLE_ID_LIST, loopID);