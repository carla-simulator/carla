function effort = getEffort(vehID, time, edgeID)
%getEffort Return the vehicle-dependent edge effort.
%   effort = getEffort(VEHID,TIME,EDGEID) Returns the edge effort for the 
%   given time as stored in the vehicle's internal container. If such a 
%   value does not exist, -1 is returned.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEffort.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_GET_VEHICLE_VARIABLE, constants.VAR_EDGE_EFFORT,...
    vehID, 1+4+1+8+1+4+length(edgeID));
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) traci.packInt32(2) ...
    uint8(sscanf(constants.TYPE_DOUBLE,'%x')) traci.packInt64(time) ...
    uint8(sscanf(constants.TYPE_STRING,'%x')) traci.packInt32(length(edgeID)) ...
    uint8(edgeID)];
result = traci.checkResult(constants.CMD_GET_VEHICLE_VARIABLE, constants.VAR_EDGE_EFFORT, vehID);
effort = result.readDouble();
