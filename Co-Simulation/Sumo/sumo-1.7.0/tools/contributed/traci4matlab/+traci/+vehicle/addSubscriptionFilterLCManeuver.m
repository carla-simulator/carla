function addSubscriptionFilterLCManeuver(direction,varargin) 
%addSubscriptionFilterLCManeuver Adds a lane-change filter to the last
%   vehicle context subscription.
%   addSubscriptionFilterLCManeuver(DIRECTION) Restricts vehicles returned
%   by the last modified vehicle context subscription to neighbor and
%   ego-lane leader and follower of the ego. DIRECTION - lane change
%   direction (in {-1=right, 1=left})
%   addSubscriptionFilterLCManeuver(LANES,NOOPPOSITE) Specifies whether vehicles
%   on opposite direction lanes shall be returned.
%   addSubscriptionFilterLCManeuver(...,DOWNSTREAMDIST) specify the range of the
%   search for surrounding vehicles along the road net in the downstream
%   direction.
%   addSubscriptionFilterLCManeuver(...,UPSTREAMDIST) specify the range of the
%   search for surrounding vehicles along the road net in the upstream
%   direction.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterLCManeuver.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'vehicle.addSubscriptionFilterLanes';
p.addRequired('direction',@isnumeric)
p.addOptional('noOpposite', 0, @(x)isnumeric(x) && (x==0 || x==1))
p.addOptional('downstreamDist', [], @isnumeric)
p.addOptional('upstreamDist', [], @isnumeric)
p.parse(direction, varargin{:})
direction = p.Results.direction;
noOpposite = p.Results.noOpposite;
downstreamDist = p.Results.downstreamDist;
upstreamDist = p.Results.upstreamDist;

if isempty(direction)
    lanes = [-1,0,1];
elseif ~(direction == -1 || direction == 1)
    warning('Ignoring lane change subscription filter ith non-neighboring lane offset direction=%s.',direction);
    return
else
    lanes = [0, direction];
end

traci.vehicle.addSubscriptionFilterLeadFollow(lanes);

if noOpposite
    traci.vehicle.addsubscriptionFilterNoOpposite()
end
if ~isempty(downstreamDist)
    traci.vehicle.addSubscriptionFilterDownstreamDistance(downstreamDist);
end
if ~isempty(upstreamDist)
    traci.vehicle.addSubscriptionFilterUpstreamDistance(upstreamDist);
end
