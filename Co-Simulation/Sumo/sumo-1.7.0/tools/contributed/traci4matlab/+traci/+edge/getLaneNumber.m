function laneNumber = getLaneNumber(edgeID)
%getLaneNumber Returns the number of lanes of this edge.
%   laneNumber = getLaneNumber(EDGEID) Returns the number of lanes on the
%   given edge.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLaneNumber.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

laneNumber = traci.edge.getUniversal(constants.VAR_LANE_INDEX, edgeID);
