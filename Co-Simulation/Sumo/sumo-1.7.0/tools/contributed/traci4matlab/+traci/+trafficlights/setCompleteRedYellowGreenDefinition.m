function setCompleteRedYellowGreenDefinition(tlsID, tls)
%setCompleteRedYellowGreenDefinition Set the complete definition of the traffic light.
%   setCompleteRedYellowGreenDefinition(TLSID,TLS) Sets the attributes of
%   trafic lights' definition including all the phase definitions. Those
%   attributes are included in the TLS parameter, which is a
%   traci.trafficlights.Logic object.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setCompleteRedYellowGreenDefinition.m 49 2018-12-27 14:08:44Z afacostag $

import traci.constants
global message
len = 1+4 + 1+4 + length(tls.programID) + 1+4 + 1+4 + 1+4; % tls parameter
for p=1:length(tls.phases)
    len = len + 1+4 + 1+8 + 1+4 + length(tls.phases{p}.state) + 1+8 + 1+8 + 1+4;
end
len = len + 1+4; % subparams
k = tls.subParameter.keys();
v = tls.subParameter.values();
for i = 1:length(tls.subParameter)
    len = len + 1+4 + length(k{i}) + 4 + length(v{i});
end
traci.beginMessage(constants.CMD_SET_TL_VARIABLE, constants.TL_COMPLETE_PROGRAM_RYG, tlsID, len);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(5)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(tls.programID)) uint8(tls.programID)];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(tls.type)];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(tls.currentPhaseIndex)];
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(length(tls.phases))];

for i=1:length(tls.phases)
    message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
        traci.packInt32(5) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
        traci.packInt64(tls.phases{i}.duration)];
    message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
        traci.packInt32(length(tls.phases{i}.state)) uint8(tls.phases{i}.state)];
    message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
        traci.packInt64(tls.phases{i}.minDur) uint8(sscanf(constants.TYPE_DOUBLE,'%x'))...
        traci.packInt64(tls.phases{i}.maxDur) uint8(sscanf(constants.TYPE_INTEGER,'%x'))...
        traci.packInt32(tls.phases{i}.next)];
end

% subparams
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(length(tls.subParameter))];
for i = 1:length(tls.subParameter)
    message.string = [message.string uint8(sscanf(constants.TYPE_STRINGLIST, '%x')) ...
        traci.packInt32(length(tls.subParameter(k{i})))];
    message.string = [message.string traci.packInt32(length(k{i})) uint8(k{i}) ...
        traci.packInt32(length(k{i})) uint8(v{i})];
end
traci.sendExact();