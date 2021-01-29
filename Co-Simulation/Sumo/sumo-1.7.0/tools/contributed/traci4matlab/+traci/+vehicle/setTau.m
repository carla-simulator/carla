function setTau(vehID, tau)
%setTau Sets the driver's reaction time in s for this vehicle.
%   setTau(VEHID,TAU) Sets the driver's reaction time in s for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setTau.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_TAU, vehID, tau);