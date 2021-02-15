function addSubscriptionFilterUpstreamDistance(dist) 
%addSubscriptionFilterUpstreamDistance Sets the upstream distance for 
%   vehicle context subscriptions.
%   addSubscriptionFilterUpstreamDistance(DIST) Sets the uptream
%   distance along the network for vehicles to be returned by the last
%   modified vehicle context subscription (call it just after subscribing).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterUpstreamDistance.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.addSubscriptionFilter(constants.FILTER_TYPE_UPSTREAM_DIST,dist);
