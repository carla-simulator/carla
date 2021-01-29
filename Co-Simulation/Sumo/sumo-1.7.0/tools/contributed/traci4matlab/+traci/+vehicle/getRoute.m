function route = getRoute(vehID)
%getRoute Get the vehicle route.
%   route = getRoute(VEHID) Returns a cell array of strings containing the 
%   ids of the edges the vehicle's route is made of.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getRoute.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
route = traci.vehicle.getUniversal(constants.VAR_EDGES, vehID);