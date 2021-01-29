function addSubscriptionFilterLeadFollow(lanes) 
%addSubscriptionFilterLeadFollow Sets leader and following restrictions to
%   vehicle context subscriptions.
%   addSubscriptionFilterLeadFollow(LANES) Restricts vehicles returned by
%   the last modified vehicle context subscription to neighbor and ego-lane
%   leader and follower of the ego.
%   Combine with: lanes-filter to restrict to one direction; distance
%   filters; vClass/vType filter.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterLeadFollow.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.addSubscriptionFilter(constants.FILTER_TYPE_LEAD_FOLLOW);
traci.addSubscriptionFilter(constants.FILTER_TYPE_LANES, lanes);
