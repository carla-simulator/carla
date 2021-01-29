function setParameter(vehID,param,value)
%setParameter Modifies the specified parameter of the vehicle.

%   Copyright 2018 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id$

import traci.constants
global message

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_PARAMETER,...
    vehID, 1 + 4 + 1 + 4 + length(param) + 1 + 4 + length(value));


message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2)];

message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) traci.packInt32(length(param)) uint8(param)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) traci.packInt32(length(value)) uint8(value)];

traci.sendExact();



