function moveToXY(vehID, edgeID, lane, x, y, angle, keepRoute)
%         Last mod: 2017/05/24 by Steffen Kutter (TU Dresden)
%
%         moveToXY(vehID, edgeID, lane, x, y, angle, keepRoute)
%
%         vehID     char    referencing vehicle        mandatory
%         edgeID    char    referencing current edge   mandatory
%         lane      int     lateral lane               mandatory
%         x         real    absolute x position        mandatory
%         y         real    absolute y position        mandatory
%         angle     real    ansolute angle in degree   additional
%         keepRoute unint8  matching behavior          additional
%
%         keepRoute=0: move to any edge, on road, route only this edge
%         keepRoute=1: closest position within the existing route is taken
%         keepRoute=2: like "0", also off road
%
%         Place vehicle at the given x,y coordinates and force it's angle to
%         the given value (for drawing). 
%         If the angle is set to INVALID_DOUBLE_VALUE, the vehicle assumes the
%         natural angle of the edge on which it is driving.
%         If keepRoute is set to 1, the closest position
%         within the existing route is taken. If keepRoute is set to 0, the vehicle may move to
%         any edge in the network but it's route then only consists of that edge.
%         If keepRoute is set to 2 the vehicle has all the freedom of keepRoute=0
%         but in addition to that may even move outside the road network.
%         edgeID and lane are optional placement hints to resovle ambiguities
%
%         
%         Further reference: <a href="matlab: 
%         web('http://www.sumo.dlr.de/userdoc/TraCI.html#TraCI_Commands')">SUMO / TraCI web site</a>.
%
%         Example:
%         veh_IDs = traci.vehicle.getIDList;
%         veh_ID=vehIDs(1);
%         angle = traci.vehicle.getAngle(veh_ID);
%         lane = traci.vehicle.getLaneID(veh_ID);
%         edgeID = traci.lane.getEdgeID(lane);
%         pos = traci.vehicle.getPosition(veh_ID);
%         traci.vehicle.moveToXY(veh_ID,edgeID,1,pos(1)+1, pos(2)+1,angle,1)
%         traci.simulationStep()

%% Main function

import traci.constants;
global message;

if nargin==5
    angle= constants.INVALID_DOUBLE_VALUE;
    keepRoute = 1;
end

if nargin==6
    keepRoute = 1;
end


traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.MOVE_TO_XY,...
    vehID,1+4+1+4+length(edgeID)+ 1 + 4 + 1 + 8 + 1 + 8 + 1 + 8 + 1 + 1);


% Number of parameters for hand over:
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(6)];
% Road edge
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(edgeID)) uint8(edgeID)];
% lateral lane number
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(lane)];
% x-coordinate
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(x)];
% y-coordinate
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(y)];

% angle
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(angle)];

% keep route as byte to uint8
message.string = [message.string uint8([sscanf(constants.TYPE_BYTE,'%x') uint8(keepRoute)])];

traci.sendExact();
