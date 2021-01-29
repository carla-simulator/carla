function sendIntCmd(cmdID, varID, objID, value)
%sendIntCmd An internal function to build a message which sends an int.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: sendIntCmd.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
traci.beginMessage(cmdID, varID, objID, 1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(value)];
traci.sendExact();