function remove(poiID, layer)
%remove Remove the poi from the SUMO network.
%   remove(POIID,LAYER) Removes the poi identified with POIID and
%   associated to the given layer. If no layer is given, it defaults to 0.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: remove.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 2
    layer = 0;
end
traci.beginMessage(constants.CMD_SET_POI_VARIABLE, constants.REMOVE, poiID, 1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) traci.packInt32(layer)];
traci.sendExact();