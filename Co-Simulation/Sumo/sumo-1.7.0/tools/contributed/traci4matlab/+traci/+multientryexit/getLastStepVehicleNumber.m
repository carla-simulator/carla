function lastStepVehicleNumber = getLastStepVehicleNumber(detID)
%getLastStepVehicleNumber Get the number vehicles.
%   lastStepVehicleNumber = getLastStepVehicleNumber(DETID) Returns the 
%   total number of vehicles for the last time step on the given 
%   multi-entry/multi-exit detector.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepVehicleNumber.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepVehicleNumber = traci.multientryexit.getUniversal(constants.LAST_STEP_VEHICLE_NUMBER, detID);