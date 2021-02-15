function addSubscriptionFilterNoOpposite() 
%addSubscriptionFilterNoOpposite Omits vehicles on other edges than the
%   ego's
%   addSubscriptionFilterNoOpposite() Omits vehicles on other edges than
%   the ego's for the last modified vehicle context subscription (call it
%   just after subscribing).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterNoOpposite.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.addSubscriptionFilter(constants.FILTER_TYPE_NOOPPOSITE);
