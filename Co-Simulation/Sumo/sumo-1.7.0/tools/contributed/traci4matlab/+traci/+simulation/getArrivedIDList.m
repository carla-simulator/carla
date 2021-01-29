function arrivedIDList = getArrivedIDList()
%getArrivedIDList Get the list of arrived vehicles.
%   arrivedIDList = getArrivedIDList() Returns a cell array of strings that
%   contains the IDs of the vehicles which arrived (have reached their 
%   destination and are removed from the road network) in this time step. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getArrivedIDList.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
arrivedIDList = traci.simulation.getUniversal(constants.VAR_ARRIVED_VEHICLES_IDS);