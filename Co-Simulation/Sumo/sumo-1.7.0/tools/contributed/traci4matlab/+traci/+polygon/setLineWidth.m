function setLineWidth(polygonID, lineWidth)
%setLineWidth Sets the line width for drawing unfilled polygon.
%   setLineWidth(POLYGONID, LINEWIDTH) Sets the line width for drawing
%   unfilled polygon

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setLineWidth.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendDoubleCmd(constants.CMD_SET_POLYGON_VARIABLE, constants.VAR_WIDTH,...
    polygonID, lineWidth);
