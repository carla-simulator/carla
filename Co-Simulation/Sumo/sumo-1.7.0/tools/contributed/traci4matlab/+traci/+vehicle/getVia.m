function via = getVia(vehID)
%getVia Get the via edges of the vehicle.
%   via = getVia(VEHID) Returns the ids of via edges for this vehicle. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getVia.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
via = traci.vehicle.getUniversal(constants.VAR_VIA, vehID);
