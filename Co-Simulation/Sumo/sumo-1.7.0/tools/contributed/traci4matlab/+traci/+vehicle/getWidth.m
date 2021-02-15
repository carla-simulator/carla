function width = getWidth(vehID)
%getWidth Returns the width in m of this vehicle.
%   width = getWidth(VEHID) Returns the width in m of this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getWidth.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
width = traci.vehicle.getUniversal(constants.VAR_WIDTH, vehID);