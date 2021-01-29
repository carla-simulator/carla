function edges = getEdges(personID, nextStageIndex)
%getEdges eturns a list of all edges in the nth next stage.
%   edges = getEdges(PERSONID) Returns a list of all edges in the current
%   stage.
%       For waiting stages this is a single edge
%       For walking stages this is the complete route
%       For driving stages this is [origin, destination]
%   edge = getEdges(PERSONID,NEXTSTAGEINDEX) Specify the stage.
%   NEXTSTAGEINDEX must be lower then value of getRemainingStages(personID)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEdges.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if nargin == 1
    nextStageIndex = 0;
end

traci.beginMessage(constants.CMD_GET_PERSON_VARIABLE,constants.VAR_EDGES,...
    personID, 1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(nextStageIndex)];

result = traci.checkResult(constants.CMD_GET_PERSON_VARIABLE,...
    constants.VAR_EDGES, personID);
edges = result.readStringList();
