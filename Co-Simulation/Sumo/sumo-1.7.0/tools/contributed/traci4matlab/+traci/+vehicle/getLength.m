function length = getLength(vehID)
%getLength Returns the length in m of the given vehicle.
%   length = getLength(VEHID) Returns the length in m of the given vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLength.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
length = traci.vehicle.getUniversal(constants.VAR_LENGTH, vehID);