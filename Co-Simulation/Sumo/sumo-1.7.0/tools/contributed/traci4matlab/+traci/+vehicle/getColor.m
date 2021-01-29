function color = getColor(vehID)
%getColor Get the color of the vehicle.
%   color = getColor(VEHID) Returns the vehicle's rgba color.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getColor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
color = traci.vehicle.getUniversal(constants.VAR_COLOR, vehID);