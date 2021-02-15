function appendWaitingStage(personID, duration, description, stopID)
%appendWaitingStage Appends a waiting stage.
%   appendWaitingStage(PERSONID,DURATION) Appends a waiting stage with
%   duration in s to the plan of the given person.
%   appendWaitingStage(...,DESCRIPTION) Provide a description.
%   appendWaitingStage(...,STOPID) Specify the stop ID.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: appendWaitingStage.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if nargin < 4
    stopID = '';
    if nargin < 3
        description = 'waiting';
    end
end

duration = duration * 1000;

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE, constants.APPEND_STAGE,...
    personID, 1+4+1+4+1+4+1+4+length(description)+1+4+length(stopID));

message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(4)];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(sscanf(constants.STAGE_WAITING,'%x'))];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(duration)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(description)) uint8(description)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(stopID)) uint8(stopID)];
traci.sendExact();
