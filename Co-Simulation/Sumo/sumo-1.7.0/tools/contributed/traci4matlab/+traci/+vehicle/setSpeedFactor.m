function setSpeedFactor(vehID, factor)
%setSpeedFactor Sets the speed factor within the last step.
%   setSpeedFactor(VEHID,FACTOR) Sets the speed factor for the named
%   vehicle within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setSpeedFactor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_SPEED_FACTOR, vehID, factor);