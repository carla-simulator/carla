function rerouteTraveltime(vehID, currentTravelTimes)
%rerouteTraveltime Compute new route based on the edges' assigned travel time.
%   rerouteTraveltime(VEHID) Computes a new route using the vehicle's 
%   internal and the global edge travel time information. Replaces the 
%   current route by the found.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: rerouteTraveltime.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
global message

persistent LAST_TRAVEL_TIME_UPDATE

if isempty(LAST_TRAVEL_TIME_UPDATE)
    LAST_TRAVEL_TIME_UPDATE = -1;
end

if nargin < 2
    currentTravelTimes = true;
end

if currentTravelTimes
    time = traci.simulation.getTime();
    if time ~= LAST_TRAVEL_TIME_UPDATE
        LAST_TRAVEL_TIME_UPDATE = time;
    end
    edges = traci.edge.getIDList;
    for i = 1:length(edges)
        traci.edge.adaptTraveltime(edges{i},traci.edge.getTraveltime(edges{i}));
    end
end

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_REROUTE_TRAVELTIME, vehID,...
    1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(0)];
traci.sendExact();
