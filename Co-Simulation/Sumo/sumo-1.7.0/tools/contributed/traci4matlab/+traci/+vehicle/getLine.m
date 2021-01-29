function line = getLine(vehID)
%getLine Get the line information of the vehicle.
%   line = getLine(VEHID) Returns the line information of this vehicle. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLine.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
line = traci.vehicle.getUniversal(constants.VAR_LINE, vehID);
