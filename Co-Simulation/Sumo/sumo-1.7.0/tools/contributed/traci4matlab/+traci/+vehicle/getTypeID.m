function typeID = getTypeID(vehID)
%getTypeID Returns the id of the type of the named vehicle.
%   typeID = getTypeID(VEHID) Returns the id of the type of the named 
%   vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getTypeID.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
typeID = traci.vehicle.getUniversal(constants.VAR_TYPE, vehID);