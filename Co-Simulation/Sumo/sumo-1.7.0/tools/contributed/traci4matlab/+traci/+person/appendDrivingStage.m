function appendDrivingStage(personID, toEdge, lines, varargin)
%appendDrivingStage Append driving stage.
%   appendDrivingStage(personID, toEdge, lines, varargin)
%   Appends a driving stage to the plan of the given person
%   The lines parameter should be a space-separated list of line ids

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: appendDrivingStage.m 48 2018-12-26 15:35:20Z afacostag $


import traci.constants
global message

p = inputParser;
p.FunctionName = 'person.appendDrivingStage';
p.addRequired('personID',@ischar)
p.addRequired('toEdge',@ischar)
p.addRequired('lines',@ischar)
p.addOptional('stopID', '', @ischar)   
p.parse(personID, toEdge, lines, varargin{:})

personID = p.Results.personID;
toEdge = p.Results.toEdge;
lines = p.Results.lines;
stopID = p.Results.stopID;



traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE, constants.APPEND_STAGE,...
    personID, 1+4+1+4+1+4+length(toEdge)+1+4+length(lines)+1+4+length(stopID));

message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(4)];

message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    uint8(sscanf(constants.STAGE_DRIVING,'%x'))];

message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(toEdge)) uint8(toEdge)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(lines)) uint8(lines)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(stopID)) uint8(stopID)];

traci.sendExact();