function addSubscriptionFilterDownstreamDistance(dist) 
%addSubscriptionFilterDownstreamDistance Sets the downstream distance for 
%   vehicle context subscriptions.
%   addSubscriptionFilterDownstreamDistance(DIST) Sets the downstream
%   distance along the network for vehicles to be returned by the last
%   modified vehicle context subscription (call it just after subscribing).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterDownstreamDistance.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.addSubscriptionFilter(constants.FILTER_TYPE_DOWNSTREAM_DIST,dist);
