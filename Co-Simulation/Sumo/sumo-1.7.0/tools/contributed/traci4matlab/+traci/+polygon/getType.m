function type = getType(polygonID)
%getType Returns the (abstract) type of the polygon.
%   type = getType(POLYGONID) Returns the (abstract) type of the polygon.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getType.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
type = traci.polygon.getUniversal(constants.VAR_TYPE, polygonID);