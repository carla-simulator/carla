function [vehicleID, dist] = getLeader(vehID,dist)
%getLeader Return the leading vehicle id and distance to ir.
% [vehicleID, dist] = getLeader(VEHID,DIST) Return the leading vehicle id 
% together with the distance.
% The DIST parameter defines the maximum lookahead, 0 calculates a 
% lookahead from the brake gap.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLeader.m 48 2018-12-26 15:35:20Z afacostag $

if nargin < 2
    dist = 0;
end

import traci.constants
global message
traci.beginMessage(constants.CMD_GET_VEHICLE_VARIABLE, constants.VAR_LEADER,...
    vehID, 1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x'))...
    traci.packInt64(dist)];

[vehicleID, dist] = traci.vehicle.readLeader(traci.checkResult(...
    constants.CMD_GET_VEHICLE_VARIABLE, constants.VAR_LEADER, vehID));
    