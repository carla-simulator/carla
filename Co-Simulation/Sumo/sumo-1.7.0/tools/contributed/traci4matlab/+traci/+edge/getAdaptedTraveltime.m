function adaptedTraveltime = getAdaptedTraveltime(edgeID, time)
%getAdaptedTraveltime Return the travel time value.
%   adaptedTraveltime = getAdaptedTraveltime(EDGEID,TIME) Return the travel
%   time value (in s) used for (re-)routing which is valid on the specified
%   edge at the given time.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   $Id: getAdaptedTraveltime.m 48 2018-12-26 15:35:20Z afacostag $

global message
import traci.constants
traci.beginMessage(constants.CMD_GET_EDGE_VARIABLE,...
    constants.VAR_EDGE_TRAVELTIME, edgeID, 1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x'))...
    traci.packInt64(time)];
result = traci.checkResult(constants.CMD_GET_EDGE_VARIABLE,...
    constants.VAR_EDGE_TRAVELTIME, edgeID);
adaptedTraveltime = result.readInt();