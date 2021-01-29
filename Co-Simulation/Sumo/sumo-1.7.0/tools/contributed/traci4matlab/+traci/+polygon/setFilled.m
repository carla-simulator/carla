function setFilled(polygonID, filled)
%setFilled Sets the filled status of the polygon.
%   setFilled(POLYGONID, FILLED) Sets the filled status of the polygon.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setFilled.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendIntCmd(constants.CMD_SET_POLYGON_VARIABLE, constants.VAR_FILL,...
    polygonID, int32(filled));
