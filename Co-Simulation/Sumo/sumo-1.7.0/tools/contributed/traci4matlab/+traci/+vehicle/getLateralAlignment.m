function lateralAlignment = getLateralAlignment(vehID)
%getLateralAlignment Get the lateral alignment.
%   lateralAlignment = getLateralAlignment(VEHID) Returns The preferred lateral
%   alignment of the vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLateralAlignment.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lateralAlignment = traci.vehicle.getUniversal(constants.VAR_LATALIGNMENT, vehID);
