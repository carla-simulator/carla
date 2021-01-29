function setRouteID(vehID, routeID)
%setRouteID Sets the id of the route for the named vehicle.
%   setRouteID(VEHID,ROUTEID) Sets the id of the route for the named 
%   vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setRouteID.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_ROUTE_ID, vehID, routeID);