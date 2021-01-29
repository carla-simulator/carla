function addSubscriptionFilterVClass(vClasses) 
%addSubscriptionFilterVClass Restricts vehicles returned by the last
%   modified vehicle context subscription to vehicles of the given classes.
%   addSubscriptionFilterVClass(VCLASSES) Restricts vehicles returned by
%   the last modified vehicle context subscription to vehicles of the given
%   classes.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addSubscriptionFilterVClass.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.addSubscriptionFilter(constants.FILTER_TYPE_VCLASS,vClasses);
