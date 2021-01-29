function setRoutingMode(vehID, routingMode)
%setRoutingMode Sets the current routing mode.
%   sets the current routing mode:
%       tc.ROUTING_MODE_DEFAULT    : use weight storages and fall-back to
%       edge speeds (default)
%       tc.ROUTING_MODE_AGGREGATED : use global smoothed travel times from
%       device.rerouting

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setRoutingMode.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendIntCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_ROUTING_MODE, vehID, routingMode);
