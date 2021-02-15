function changeLane(vehID, laneIndex, duration)
%changeLane Make the vehicle to switch the lane.
%   changeLane(VEHID,LANEINDEX,DURATION) Makes the vehicle with ID VEHID to
%   change the lane to the specified in the index LANEINDEX for the given
%   time duration.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: changeLane.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
global message

if duration > 1000
    warning('API change now handles duration as floating point seconds');
end

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_CHANGELANE,...
    vehID, 1+4+1+1+1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2) uint8([sscanf(constants.TYPE_BYTE,'%x') ...
    laneIndex sscanf(constants.TYPE_DOUBLE,'%x')]) ...
    traci.packInt64(duration)];
traci.sendExact();
