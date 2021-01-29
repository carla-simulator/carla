function shape = getShape(polygonID)
%getShape Get the shape of the polygon.
%   shape = getShape(POLYGONID) Returns the shape of the given polygon, 
%   which is a cell array containing 2-dimensional vectors that represent
%   the x and y coordinates of the points that define the shape of the
%   polygon.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getShape.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
shape = traci.polygon.getUniversal(constants.VAR_SHAPE, polygonID);