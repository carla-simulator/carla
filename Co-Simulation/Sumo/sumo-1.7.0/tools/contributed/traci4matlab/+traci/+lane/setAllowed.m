function setAllowed(laneID, allowedClasses)
%setAllowed Set the allowed vehicle classes in the lane.
%   setAllowed(LANEID,ALLOWEDCLASSES) Sets a list of allowed vehicle 
%   classes contained in a cell array of strings. Setting an empty list 
%   means all vehicles are allowed.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setAllowed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_SET_LANE_VARIABLE, constants.LANE_ALLOWED,...
laneID, 1+4+sum(cellfun('length', allowedClasses))+4*length(allowedClasses));
message.string = [message.string uint8(sscanf(constants.TYPE_STRINGLIST,'%x')) ...
    traci.packInt32(length(allowedClasses))];
for i=1:length(allowedClasses)
    message.string = [message.string traci.packInt32(length(...
        allowedClasses{i})) uint8(allowedClasses{i})];
end
traci.sendExact();