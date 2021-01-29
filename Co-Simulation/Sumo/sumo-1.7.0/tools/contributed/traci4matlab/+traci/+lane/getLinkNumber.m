function linkNumber = getLinkNumber(laneID)
%getLinkNumber Returns the number of connections to successive lanes.
%   linkNumber = getLinkNumber(LANEID) Returns the number of connections to
%   successive lanes.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLinkNumber.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
linkNumber = traci.lane.getUniversal(constants.LANE_LINK_NUMBER, laneID);