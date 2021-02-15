function addSubscriptionFilterLanes(lanes,varargin) 
%addSubscriptionFilterLanes Adds a lane-filter to the last vehicle context
%   subscription.
%   addSubscriptionFilterLanes(LANES) Adds a lane-filter to the last
%   modified vehicle context subscription (call it just after subscribing).
%   LANES is an array of relative lane indices (-1 -> right neighboring
%   lane of the ego, 0 -> ego lane, etc.)
%   addSubscriptionFilterLanes(LANES,NOOPPOSITE) Specifies whether vehicles
%   on opposite direction lanes shall be returned.
%   addSubscriptionFilterLanes(...,DOWNSTREAMDIST) specify the range of the
%   search for surrounding vehicles along the road net in the downstream
%   direction.
%   addSubscriptionFilterLanes(...,UPSTREAMDIST) specify the range of the
%   search for surrounding vehicles along the road net in the upstream
%   direction.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterLanes.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'vehicle.addSubscriptionFilterLanes';
p.addRequired('lanes',@isnumeric)
p.addOptional('noOpposite', 0, @(x)isnumeric(x) && (x==0 || x==1))
p.addOptional('downstreamDist', [], @isnumeric)
p.addOptional('upstreamDist', [], @isnumeric)
p.parse(lanes, varargin{:})
lanes = p.Results.lanes;
noOpposite = p.Results.noOpposite;
downstreamDist = p.Results.downstreamDist;
upstreamDist = p.Results.upstreamDist;

traci.addSubscriptionFilter(constants.FILTER_TYPE_LANES,lanes);
if noOpposite
    traci.vehicle.addSubscriptionFilterNoOpposite()
end
if ~isempty(downstreamDist)
    traci.vehicle.addSubscriptionFilterDownstreamDistance(downstreamDist);
end
if ~isempty(upstreamDist)
    traci.vehicle.addSubscriptionFilterUpstreamDistance(upstreamDist);
end
