function speed = getSpeed(vehID)
%getSpeed Get the vehicle speed.
%   speed = getSpeed(VEHID) Returns the speed in m/s of the named vehicle 
%   within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSpeed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
speed = traci.vehicle.getUniversal(constants.VAR_SPEED, vehID);