function parameter = getParameter(vehID,param)
%getParameter Returns the specified parameter of the vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getParameter.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message

traci.beginMessage(constants.CMD_GET_VEHICLE_VARIABLE, constants.VAR_PARAMETER, vehID,  1 + 4 + length(param));

message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) traci.packInt32(length(param)) uint8(param)];
result = traci.checkResult(constants.CMD_GET_VEHICLE_VARIABLE, constants.VAR_PARAMETER, vehID);

parameter=result.readString();



