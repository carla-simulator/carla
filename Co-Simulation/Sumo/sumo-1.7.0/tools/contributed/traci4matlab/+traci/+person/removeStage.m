function removeStage(personID, nextStageIndex)
%removeStage Removes the nth next stage.
%   removeStage(PERSONID,NEXTSTAGEINDEX) Removes the nth next stage.
%   NEXTSTAGEINDEX must be lower than value of getRemainingStages(personID)
%   NEXTSTAGEINDEX 0 immediately aborts the current stage and proceeds to
%   the next stage

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: removeStage.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE, constants.REMOVE_STAGE,...
    personID, 1+4);

message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(nextStageIndex)];
traci.sendExact();
