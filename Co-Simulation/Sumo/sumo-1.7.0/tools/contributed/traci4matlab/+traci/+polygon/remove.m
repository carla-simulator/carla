function remove(polygonID, layer)
%remove Remove the polygon from the SUMO network.
%   remove(POLYGONID,LAYER) Removes the polygon identified with POLYGONID 
%   and associated to the given layer. If no layer is given, it defaults to 0.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: remove.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 2
    layer = 0;
end
traci.beginMessage(constants.CMD_SET_POLYGON_VARIABLE, constants.REMOVE, polygonID, 1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) traci.packInt32(layer)];
traci.sendExact();