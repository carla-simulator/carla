function distanceRoad = getDistanceRoad(edgeID1, pos1, edgeID2, pos2, isDriving)
%getDistanceRoad Get the distance between two relative-to-edge positions.
%   distanceRoad = getDistanceRoad(EDGEID1,POS1,EDGEID2,POS2) Returns the 
%   distance between the points defined by the relative-to-edge positions
%   (EDGEID1,POS1) and (EDGEID2,POS2).
%   distance2D = getDistance2D(...,ISDRIVING) Specify wether the air or the
%   driving distance shall be computed.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getDistanceRoad.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 5
    isDriving = false;
end

distType = sscanf(constants.REQUEST_AIRDIST,'%x');

if isDriving
    distType = sscanf(constants.REQUEST_DRIVINGDIST,'%x');
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.DISTANCE_REQUEST,...
    '', 1+4 + 1+4+length(edgeID1)+8+1 + 1+4+length(edgeID2)+8+1 + 1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(3)];
message.string = [message.string uint8(sscanf(constants.POSITION_ROADMAP,'%x')) ...
    traci.packInt32(length(edgeID1)) uint8(edgeID1)];
message.string = [message.string traci.packInt64(pos1) uint8([0 sscanf(constants.POSITION_ROADMAP,'%x')]) ...
    traci.packInt32(length(edgeID2)) uint8(edgeID2)];
message.string = [message.string traci.packInt64(pos2) uint8([0 distType])];
result = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.DISTANCE_REQUEST, '');
distanceRoad = result.readDouble;