function sendStringCmd(cmdID, varID, objID, value)
%sendStringCmd An internal function to build a message which sends a string.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: sendStringCmd.m 48 2018-12-26 15:35:20Z afacostag $

global message
import traci.constants

% Construct the TraCI message
traci.beginMessage(cmdID, varID, objID, 1+4+length(value))
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(value)) uint8(value)];

% Send the TraCI message
traci.sendExact();
