function height = getHeight(vehID)
%getHeight Get the height of the vehicle.
%   height = getHeight(VEHID) Returns the height in m of this vehicle. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getHeight.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
height = traci.vehicle.getUniversal(constants.VAR_HEIGHT, vehID);
