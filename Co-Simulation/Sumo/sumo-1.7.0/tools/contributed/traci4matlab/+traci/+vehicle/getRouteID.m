function routeID = getRouteID(vehID)
%getRouteID Returns the id of the route of the named vehicle.
%   routeID = getRouteID(VEHID) Returns the id of the route of the named 
%   vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getRouteID.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
routeID = traci.vehicle.getUniversal(constants.VAR_ROUTE_ID, vehID);