function setHeight(vehID, height)
%setHeight Sets the height in m for this vehicle.
%   setHeight(VEHID,WIDTH) Sets the height in m for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setHeight.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_HEIGHT, vehID, height);
