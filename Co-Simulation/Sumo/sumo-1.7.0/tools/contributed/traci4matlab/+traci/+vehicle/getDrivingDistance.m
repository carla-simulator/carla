function drivingDistance = getDrivingDistance(vehID, edgeID, pos, laneID)
%getDrivingDistance Returns the driving distance to relative coordinate.
%   drivingDistance = getDrivingDistance(VEHID,EDGEID,POS,LANEID) Returns 
%   the driving distance from the current position to the given in POS. POS
%   is a position relative to the edge identified by EDGEID and the lane
%   index specified in LANEID. If no LANEID is given, it defaults to zero.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getDrivingDistance.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 4
    laneID=0;
end
traci.beginMessage(constants.CMD_GET_VEHICLE_VARIABLE, constants.DISTANCE_REQUEST,...
    vehID, 1+4+1+4+length(edgeID) + 8+1+1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) traci.packInt32(2) ...
    uint8(sscanf(constants.POSITION_ROADMAP,'%x')) traci.packInt32(length(edgeID)) uint8(edgeID)];
message.string = [message.string traci.packInt64(pos) uint8([laneID sscanf(constants.REQUEST_DRIVINGDIST,'%x')])];
result = traci.checkResult(constants.CMD_GET_VEHICLE_VARIABLE, constants.DISTANCE_REQUEST, vehID);
drivingDistance = result.readDouble();