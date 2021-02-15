function accel = getAcceleration(vehID)
%getAcceleration Get the acceleration of the vehicle.
%   accel = accel = getAcceleration(VEHID) Returns the acceleration in
%   m/s^2 of this vehicle in the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getAcceleration.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
accel = traci.vehicle.getUniversal(constants.VAR_ACCELERATION, vehID);
