function addSubscriptionFilterCFManeuver(downstreamDist,upstreamDist) 
%addSubscriptionFilterCFManeuver Restricts vehicles returned by the last
%   modified vehicle context subscription to leader and follower of the ego.
%   addSubscriptionFilterCFManeuver() Restricts vehicles returned by the
%   last modified vehicle context subscription to leader and follower of
%   the ego.
%   addSubscriptionFilterCFManeuver(DOWNSTREAMDIST) Specify the range of
%   the search in the downstream direction.
%   addSubscriptionFilterCFManeuver(UPSTREAMDIST) Specify the range of
%   the search in the upstream direction.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterCFManeuver.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

if nargin < 2
    upstreamDist = [];
    if nargin < 1
        downstreamDist = [];
    end
end

traci.vehicle.addSubscriptionFilterLeadFollow([0]);
if ~isempty(downstreamDist)
    traci.vehicle.addSubscriptionFilterDownstreamDistance(downstreamDist);
end
if ~isempty(upstreamDist)
    traci.vehicle.addSubscriptionFilterUpstreamDistance(upstreamDist);
end
