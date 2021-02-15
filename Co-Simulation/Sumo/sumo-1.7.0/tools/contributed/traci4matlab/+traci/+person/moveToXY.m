function moveToXY(personID, edgeID, x, y, angle, keepRoute)
%moveToXY Place person at the given x,y coordinates.
%   moveToXY(PERSONID,EDGEID,X,Y) Place person at the given x,y
%   coordinates. the closest position within the existing route is taken.
%   EDGEID is an optional placement hint to resolve ambiguities
%   moveToXY(..,ANGLE) Force the person's angle to the given value
%   (for drawing).
%   moveToXY(..,KEEPROUTE) If keepRoute is set to 0, the vehicle may move
%   to any edge in the network but it's route then only consists of that
%   edge. If keepRoute is set to 2 the person has all the freedom of
%   keepRoute=0 but in addition to that may even move outside the road
%   network.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: moveToXY.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if nargin < 6
    keepRoute = 1;
    if nargin < 5
        angle = constants.INVALID_DOUBLE_VALUE;
    end
end

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE,...
    constants.MOVE_TO_XY, personID, 1+4+1+4+length(edgeID)+1+8+1+8+1+8+1+1);

message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(5)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(edgeID)) uint8(edgeID)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(x)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(y)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(angle)];
message.string = [message.string uint8(sscanf(constants.TYPE_BYTE,'%x')) ...
    uint8(keepRoute)];
traci.sendExact();
