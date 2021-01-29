function subscribeLeader(vehID, varargin)
%subscribeLeader Subscribe to the leading vehicle.
%   subscribeLeader(VEHID) Subscribe for the leading vehicle id together
%   with the distance. The lookahead is calculated from the brake gap.
%   subscribeLeader(VEHID,DIST) Specify the lookahead distance.
%   subscribeLeader(...,BEGIN) Specify the starting time the subscription
%   is made.
%   subscribeLeader(...,END) Specify the ending time the subscription is
%   made.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: subscribeLeader.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

dist = 0;
begin = 0;
endTime = 2^31-1;

if nargin > 1
    dist = varargin{1};
    if nargin > 2
        begin = varargin{2};
        if nargin > 3
            endTime = varargin{3};
        end
    end
end

traci.subscribe(traci.constants.CMD_SUBSCRIBE_VEHICLE_VARIABLE, begin,...
    endTime, vehID, {traci.constants.VAR_LEADER},...
    containers.Map({traci.constants.VAR_LEADER},{[uint8(sscanf(traci.constants.TYPE_DOUBLE,'%x')) traci.packInt64(dist)]}));
