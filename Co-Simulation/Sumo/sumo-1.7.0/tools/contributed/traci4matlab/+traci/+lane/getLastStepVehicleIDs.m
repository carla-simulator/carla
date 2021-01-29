function lastStepVehicleIDs = getLastStepVehicleIDs(laneID)
%getLastStepVehicleIDs Get the IDs of the vehicles in the lane.
%   lastStepVehicleIDs = getLastStepVehicleIDs(LANEID) Returns cell array 
%   of strings containing the IDs of the vehicles for the last time step 
%   on the given lane.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepVehicleIDs.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepVehicleIDs = traci.lane.getUniversal(constants.LAST_STEP_VEHICLE_ID_LIST, laneID);