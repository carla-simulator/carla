function [state, stateTraCI] = getLaneChangeState(vehID, direction)
%getLaneChangeState Return the lane change state for the vehicle.
%   lcState = getLaneChangeState(VEHID) Return the lane change state for
%   the vehicle

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLaneChangeState.m 54 2019-01-03 15:41:54Z afacostag $

global message
import traci.constants
traci.beginMessage(constants.CMD_GET_VEHICLE_VARIABLE, constants.CMD_CHANGELANE,...
    vehID, 1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(direction)];
result = traci.checkResult(constants.CMD_GET_VEHICLE_VARIABLE,...
    constants.CMD_CHANGELANE, vehID);

result.readInt();
result.read(1);
state = result.readInt();
result.read(1);
stateTraCI = result.readInt();
