function setVia(vehID, edgeList)
%setVia Changes the via edges to the given edges list.
%   setVia(VEHID,EDGELIST) changes the via edges to the given edges list
%   (to be used during subsequent rerouting calls).
%   Note: a single edgeId as argument is allowed as shorthand for a list of
%   length 1

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setVia.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

len = 0;
for i = 1:length(edgeList)
    len = len + length(edgeList{i});
end

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_VIA, vehID,...
    1+4+len+4*length(edgeList));
message.string = [message.string uint8(sscanf(constants.TYPE_STRINGLIST,'%x')) ...
    traci.packInt32(length(edgeList))];
for i = 1:length(edgeList)
    message.string = [message.string traci.packInt32(length(edgeList{i})) ...
        uint8(edgeList{i})];
end
traci.sendExact();
