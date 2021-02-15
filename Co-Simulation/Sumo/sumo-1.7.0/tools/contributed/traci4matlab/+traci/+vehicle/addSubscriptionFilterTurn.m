function addSubscriptionFilterTurn(downStreamDist,upstreamDist) 
%addSubscriptionFilterTurn Restricts vehicles returned by the last modified
%   vehicle context subscription to foes on an upcoming junction
%   addSubscriptionFilterTurn() Restricts vehicles returned by the last
%   modified vehicle context subscription to foes on an upcoming junction
%   addSubscriptionFilterTurn(DOWNSTREAMDIST) Specify the range of
%   the search in the downstream direction.
%   addSubscriptionFilterTurn(UPSTREAMDIST) Specify the range of
%   the search in the upstream direction.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterTurn.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

if nargin < 2
    upstreamDist = [];
    if nargin < 1
        downstreamDist = [];
    end
end

traci.addSubscriptionFilter(constants.FILTER_TYPE_TURN);
if ~isempty(downstreamDist)
    traci.vehicle.addSubscriptionFilterDownstreamDistance(downstreamDist);
end
if ~isempty(upstreamDist)
    traci.vehicle.addSubscriptionFilterUpstreamDistance(upstreamDist);
end
