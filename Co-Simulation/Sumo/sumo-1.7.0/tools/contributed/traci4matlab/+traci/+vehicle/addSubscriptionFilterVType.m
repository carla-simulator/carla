function addSubscriptionFilterVType(vTypes) 
%addSubscriptionFilterVType Restricts vehicles returned by the last
%   modified vehicle context subscription to vehicles of the given types.
%   addSubscriptionFilterVType(VTYPES) Restricts vehicles returned by
%   the last modified vehicle context subscription to vehicles of the given
%   types.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterVType.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.addSubscriptionFilter(constants.FILTER_TYPE_VTYPE,vTypes);
