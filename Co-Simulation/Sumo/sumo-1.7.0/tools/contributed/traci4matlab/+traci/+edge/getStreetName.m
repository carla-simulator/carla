function streetName = getStreetName(edgeID)
%getStreetName Returns the street name of this edge.
%   streetName = getStreetName(EDGEID) Returns street name on the given
%   edge.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStreetName.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

streetName = traci.edge.getUniversal(constants.VAR_NAME, edgeID);
