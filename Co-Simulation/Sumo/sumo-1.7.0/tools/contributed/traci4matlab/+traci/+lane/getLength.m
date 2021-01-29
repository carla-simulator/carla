function length = getLength(laneID)
%getLength Returns the length of the lane in m.
%   length = getLength(LANEID) Returns the length of the lane in m.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLength.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
length = traci.lane.getUniversal(constants.VAR_LENGTH, laneID);