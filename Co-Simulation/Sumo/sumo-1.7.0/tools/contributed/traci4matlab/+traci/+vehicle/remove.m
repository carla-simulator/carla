function remove(vehID, reason)
%remove Remove vehicle.
%   remove(VEHID,REASON)  Remove vehicle with the given ID for the given 
%   reason. Reasons are defined in traci.constants and start with REMOVE_

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: remove.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants

if nargin < 2
    reason = sscanf(constants.REMOVE_VAPORIZED,'%x');
end

traci.sendByteCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.REMOVE, vehID, reason);