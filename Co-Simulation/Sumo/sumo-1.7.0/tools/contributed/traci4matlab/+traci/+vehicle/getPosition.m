function position = getPosition(vehID)
%getPosition Returns the position of the named vehicle within the last step.
%   position = getPosition(VEHID) Returns the x,y position of the named 
%   vehicle within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPosition.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
position = traci.vehicle.getUniversal(constants.VAR_POSITION, vehID);