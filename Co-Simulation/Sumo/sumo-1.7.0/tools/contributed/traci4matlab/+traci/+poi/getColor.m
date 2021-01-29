function color = getColor(poiID)
%getColor Get the color of the poi.
%   color = getColor(POIID) Returns the rgba color of the given poi.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getColor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
color = traci.poi.getUniversal(constants.VAR_COLOR, poiID);