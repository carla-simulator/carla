function IDCount = getIDCount()
%getIDCount Get the number of edges in the SUMO network.  

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getIDCount.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
IDCount = traci.edge.getUniversal(constants.ID_COUNT, '');