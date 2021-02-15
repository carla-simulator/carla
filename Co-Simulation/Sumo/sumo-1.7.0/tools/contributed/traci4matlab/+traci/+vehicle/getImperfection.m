function imperfection = getImperfection(vehID)
%getImperfection Returns driver's imperfection.
%   imperfection = getImperfection(VEHID) Returns the driver's imperfection
%   (dawdling) [0,1]

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getImperfection.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
imperfection = traci.vehicle.getUniversal(constants.VAR_IMPERFECTION, vehID);