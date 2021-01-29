function changeLaneRelative(vehID, left, duration)
%changeLaneRelative Forces a relative lane change.
%   changeLaneRelative(VEHID,LEFT,DURATION) Forces a relative lane change;
%   if successful, the lane will be chosen for the given amount of time
%   (in s).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: changeLaneRelative.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if duration > 1000
    warning('API change now handles duration as floating point seconds.')
end

if left > 0
    laneIndex = left;
else
    laneIndex = 0;
end

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_CHANGELANE,...
    vehID, 1+4+1+1+1+8+1+1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(3) uint8([sscanf(constants.TYPE_BYTE,'%x') ...
    laneIndex sscanf(constants.TYPE_DOUBLE,'%x')]) ...
    traci.packInt64(duration) uint8(sscanf(constants.TYPE_BYTE,'%x')) ...
    uint8(1)];
traci.sendExact();
