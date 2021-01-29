function setDisallowed(laneID, disallowedClasses)
%setDisallowed Set the disallowed vehicle classes in the lane.
%   setDisallowed(LANEID,DISALLOWEDCLASSES) Sets a list of disallowed 
%   vehicle classes contained in a cell array of strings.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setDisallowed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_SET_LANE_VARIABLE, constants.LANE_DISALLOWED,...
laneID, 1+4+sum(cellfun('length', disallowedClasses))+4*length(disallowedClasses));
message.string = [message.string uint8(sscanf(constants.TYPE_STRINGLIST,'%x')) ...
    traci.packInt32(length(disallowedClasses))];
for i=1:length(disallowedClasses)
    message.string = [message.string traci.packInt32(length(...
        disallowedClasses{i})) uint8(disallowedClasses{i})];
end
traci.sendExact();