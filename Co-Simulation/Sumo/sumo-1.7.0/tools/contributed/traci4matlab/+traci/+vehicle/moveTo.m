function moveTo(vehID, laneID, pos)
%moveTo Commands the vehicle to move to the specified position.
%   moveTo(VEHID,LANEID,POS) Commands the vehicle to move to the specified
%   position on the given lane.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: moveTo.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
global message

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_MOVE_TO, vehID,...
    1+4+1+4+length(laneID)+1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(laneID)) uint8(laneID)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(pos)];
traci.sendExact();
