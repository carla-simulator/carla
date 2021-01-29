function lastStepOccupancy = getLastStepOccupancy(loopID)
%getLastStepOccupancy Get the percentage of time the loop was occupied.
%   lastStepOccupancy = getLastStepOccupancy(LOOPID) Returns the percentage
%   of time the detector was occupied by a vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepOccupancy.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepOccupancy = traci.inductionloop.getUniversal(constants.LAST_STEP_OCCUPANCY, loopID);