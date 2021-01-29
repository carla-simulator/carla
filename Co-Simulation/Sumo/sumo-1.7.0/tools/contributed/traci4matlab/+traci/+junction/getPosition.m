function position = getPosition(junctionID)
%getPosition Get the position of the junction.
%   position = getPosition(JUNCTIONID) Returns the coordinates of the 
%   center of the junction.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPosition.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
position = traci.junction.getUniversal(constants.VAR_POSITION, junctionID);