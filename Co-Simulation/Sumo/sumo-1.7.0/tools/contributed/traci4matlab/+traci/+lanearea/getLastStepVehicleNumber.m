function lastStepVehicleNumber = getLastStepVehicleNumber(detID)
%getLastStepVehicleNumber Get the number of the vehicles in the detector.
%   lastStepVehicleNumber = getLastStepVehicleNumber(DETID) Returns the
%   number of vehicles that were on the named detector within the last
%   simulation step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepVehicleNumber.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
lastStepVehicleNumber = traci.lanearea.getUniversal(constants.LAST_STEP_VEHICLE_NUMBER, detID);