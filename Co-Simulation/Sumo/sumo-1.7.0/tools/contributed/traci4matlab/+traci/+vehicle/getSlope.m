function slope = getSlope(vehID)
%getSlope Get the slope of the vehicle.
%   slope = getSlope(VEHID) Returns the slope at the current position of
%   the vehicle in degrees

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSlope.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
slope = traci.vehicle.getUniversal(constants.VAR_SLOPE, vehID);
