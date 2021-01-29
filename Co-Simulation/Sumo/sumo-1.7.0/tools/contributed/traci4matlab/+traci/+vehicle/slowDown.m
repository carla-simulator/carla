function slowDown(vehID, speed, duration)
%slowDown Reduces the speed of the vehicle.
%   slowDown(VEHID,SPEED,DURATION) Reduces the speed of the vehicle to the 
%   given for the given amount of time.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: slowDown.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants

if duration >= 1000
    warning('API change now handles duration as floating point seconds');
end

global message
traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_SLOWDOWN,...
    vehID, 1+4+1+8+1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(speed) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(duration)];
traci.sendExact();