function isFilled = getFilled(polygonID)
%getFilled Returns whether the polygon is filled.
%   isFilled = getFilled(POLYGONID) Returns whether the polygon is filled.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getFilled.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
isFilled = traci.polygon.getUniversal(constants.VAR_FILL, polygonID);
