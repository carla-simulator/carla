function routingMode = getRoutingMode(vehID)
%getRoutingMode Returns the current routing mode.
%   routingMode = getRoutingMode(VEHID) returns the current routing mode:
%       tc.ROUTING_MODE_DEFAULT    : use weight storages and fall-back to
%       edge speeds (default)
%       tc.ROUTING_MODE_AGGREGATED : use global smoothed travel times from
%       device.rerouting

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getRoutingMode.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
routingMode = traci.vehicle.getUniversal(constants.VAR_ROUTING_MODE, vehID);
