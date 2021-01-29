function saveState(fileName)
%saveState
%   saveState(FILENAME)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: saveState.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

traci.beginMessage(constants.CMD_SET_SIM_VARIABLE, constants.CMD_SAVE_SIMSTATE,...
    '', 1+4+length(fileName));
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(fileName)) uint8(fileName)];
traci.sendExact();
