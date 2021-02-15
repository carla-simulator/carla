function lineWidth = getLineWidth(polygonID)
%getLineWidth Returns drawing width of unfilled polygon.
%   lineWidth = getLineWidth(POLYGONID) Returns drawing width of unfilled
%   polygon.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLineWidth.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
lineWidth = traci.polygon.getUniversal(constants.VAR_WIDTH, polygonID);
