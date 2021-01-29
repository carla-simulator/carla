function appendWalkingStage(personID, edges, arrivalPos, duration, speed,...
    stopID)
%appendWalkingStage Appends a walking stage.
%   appendWalkingStage(PERSONID,EDGES,ARRIVALPOS) Appends a walking stage
%   to the plan of the given person.
%   appendWalkingStage(...,DURATION) Specify the duration. The walking
%   speed is computed from the DURATION parameter (in s) or taken from the
%   type of the person
%   appendWalkingStage(...,SPEED) Specify the speed.
%   appendWalkingStage(...,STOPID) Specify the stop ID.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: appendWalkingStage.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if nargin < 6
    stopID= '';
    if nargin < 5
        speed = -1;
        if nargin < 4
            duration = -1;
        end
    end
end

if ~isempty(duration)
    duration = duration * 1000;
end

len = 0;
for i = 1:length(edges)
    len = len + length(edges{i});
end

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE, constants.APPEND_STAGE,...
    personID, 1+4+1+4+1+4+len+4*length(edges)+1+8+1+4+1+8+1+4+length(stopID));

message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(6)];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(sscanf(constants.STAGE_WALKING,'%x'))];
message.string = [message.string uint8(sscanf(constants.TYPE_STRINGLIST,'%x')) ...
    traci.packInt32(length(edges))];
for i = 1:length(edges)
    message.string = [message.string traci.packInt32(length(edges{i})) ...
        uint8(edges{i})];
end

message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(length(arrivalPos))];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(duration)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(length(speed))];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(stopID)) uint8(stopID)];
traci.sendExact();
