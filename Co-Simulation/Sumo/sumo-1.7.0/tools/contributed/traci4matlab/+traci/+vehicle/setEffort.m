function setEffort(vehID, edgeID, effort, begTime, endTime)
%setEffort Inserts the information about the effort of the given edge.
%   setEffort(VEHID,BEGTIME,ENDTIME,EDGEID,EFFORT) Inserts the information 
%   about the effort of edge EDGEID valid from BEGTIME time to ENDTIME time
%   into the vehicle's internal edge weights container.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setEffort.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
global message

if nargin < 5
    endTime = [];
    if nargin < 4
        begTime = [];
        if nargin < 3
            effort = [];
        end
    end
end

if isempty(effort)
    traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE,...
        constants.VAR_EDGE_EFFORT, vehID,...
        1+4+1+4+length(edgeID));
    message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
        traci.packInt32(1)];
    message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
        traci.packInt32(length(edgeID)) uint8(edgeID)];
    traci.sendExact();
elseif isempty(begTime)
    traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE,...
        constants.VAR_EDGE_EFFORT, vehID,...
        1+4+1+4+length(edgeID)+1+8);
        message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
            traci.packInt32(2)];
        message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
            traci.packInt32(length(edgeID)) uint8(edgeID)];
        message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
             traci.packInt64(effort)];
        traci.sendExact(); 
else
    traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE,...
        constants.VAR_EDGE_EFFORT, vehID,...
        1+4+1+8+1+8+1+4+length(edgeID)+1+8);
    message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
        traci.packInt32(4) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
        traci.packInt64(begTime) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
        traci.packInt64(endTime)];
    message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
        traci.packInt32(length(edgeID)) uint8(edgeID)];
    message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
        traci.packInt64(effort)];
    traci.sendExact();
end
