function effort = getEffort(edgeID, time)
%getEffort Get the effort used for (re-)routing.
%   effort = getEffort(EDGEID,TIME) Returns the effort value used for 
%   (re-)routing which is valid on the edge at the given time.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEffort.m 48 2018-12-26 15:35:20Z afacostag $

global message
import traci.constants
traci.beginMessage(constants.CMD_GET_EDGE_VARIABLE, constants.VAR_EDGE_EFFORT,...
    edgeID, 1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x'))...
    traci.packInt64(time)];
result = traci.checkResult(constants.CMD_GET_EDGE_VARIABLE,...
    constants.VAR_EDGE_EFFORT, edgeID);
effort = result.readDouble();