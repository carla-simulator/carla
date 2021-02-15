function position3D = getPosition3D(vehID)
%getPosition3D Returns the position of the named vehicle within the last step.
%   position = getPosition3D(VEHID) Returns the x,y,z position of the named 
%   vehicle within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPosition3D.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
position3D = traci.vehicle.getUniversal(constants.VAR_POSITION3D, vehID);