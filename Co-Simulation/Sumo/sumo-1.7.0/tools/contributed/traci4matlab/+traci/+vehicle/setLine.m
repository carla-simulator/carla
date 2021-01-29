function setLine(vehID, line)
%setLine Sets the line information for this vehicle.
%   setLine(VEHID,LINE) Sets the line information for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setLine.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_LINE, vehID, line);
