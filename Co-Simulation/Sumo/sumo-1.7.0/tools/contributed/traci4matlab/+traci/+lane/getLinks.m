function links = getLinks(laneID)
%getLinks Returns a cell containing ids of successor lanes together with
%priority, open and foe.
%   links = getLinks(LANEID) Returns a cell containing ids of successor 
%   lanes together with priority, open and foe.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLinks.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
links = traci.lane.getUniversal(constants.LANE_LINKS, laneID);