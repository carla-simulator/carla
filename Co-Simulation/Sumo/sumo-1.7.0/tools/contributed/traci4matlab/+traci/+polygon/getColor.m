function color = getColor(polygonID)
%getColor Get the color of the polygon.
%   color = getColor(POLYGONID) Returns the rgba color of the given polygon.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getColor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
color = traci.polygon.getUniversal(constants.VAR_COLOR, polygonID);