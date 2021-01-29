function  stage = getStage(personID, varargin)
%
%   getStage(personID,nextStageIndex)Returns the type of the nth next stage
%           0 for not-yet-departed
%           1 for waiting
%           2 for walking
%           3 for driving
%         nextStageIndex 0 retrieves value for the current stage.
%         nextStageIndex must be lower then value of getRemainingStages(personID)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStage.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message

p = inputParser;
p.FunctionName = 'person.getStage';
p.addRequired('personID',@ischar)
p.addOptional('nextStageIndex', 0, @isnumeric)   
p.parse(personID, varargin{:})


personID = p.Results.personID;
nextStageIndex = p.Results.nextStageIndex;

traci.beginMessage(constants.CMD_GET_PERSON_VARIABLE, constants.VAR_STAGE,...
    personID, 1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(nextStageIndex)];

result = traci.checkResult(constants.CMD_GET_PERSON_VARIABLE, constants.VAR_STAGE, personID);

stage = result.readInt();
