function setShape(polygonID, shape)
%setShape Sets the shape of the polygon.
%   setShape(POLYGONID,SHAPE) Sets the shape of the polygon to the given. 
%   SHAPE is a cell array whose elements are 2-dimensional vectors that 
%   represent the x and y coordinates of the points that define the shape 
%   of the polygon.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setShape.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_SET_POLYGON_VARIABLE, constants.VAR_SHAPE, polygonID, 1+1+length(shape)*(8+8));
message.string = [message.string uint8([sscanf(constants.TYPE_POLYGON,'%x') length(shape)])]; 
for i=1:length(shape)
    message.string = [message.string traci.packInt64(fliplr(shape{i}))];
end
traci.sendExact();