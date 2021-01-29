function clearPending(varargin)
%clearPending
%   clearPending()
%   clearPending(ROUTEID)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: clearPending.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

routeID = '';

if nargin > 0
    routeID = varargin{1};
end

traci.beginMessage(constants.CMD_SET_SIM_VARIABLE, constants.CMD_CLEAR_PENDING_VEHICLES,...
    '', 1+4+length(routeID));
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(routeID)) uint8(routeID)];
traci.sendExact();
