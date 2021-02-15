function edgeID = getEdgeID(laneID)
%getEdgeID Returns the id of the edge the lane belongs to.
%   edgeID = getEdgeID(LANEID) Returns the id of the edge the lane belongs 
%   to.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEdgeID.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
edgeID = traci.lane.getUniversal(constants.LANE_EDGE_ID, laneID);