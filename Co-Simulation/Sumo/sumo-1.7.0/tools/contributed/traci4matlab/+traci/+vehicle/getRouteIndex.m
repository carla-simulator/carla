function routeIndex = getRouteIndex(vehID)
%getRouteIndex Returns the index of the edge on the route route of the
%   named vehicle.
%   routeID = getRouteIndex(VEHID) Returns the index of the current edge
%   within the vehicles route or -1 if the vehicle has not yet departed.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getRouteIndex.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
routeIndex = traci.vehicle.getUniversal(constants.VAR_ROUTE_INDEX, vehID);
