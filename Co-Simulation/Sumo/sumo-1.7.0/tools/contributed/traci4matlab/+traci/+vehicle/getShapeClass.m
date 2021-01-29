function shapeClass = getShapeClass(vehID)
%getShapeClass Returns the shape class of this vehicle.
%   shapeClass = getShapeClass(VEHID) Returns the shape class of this 
%   vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getShapeClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
shapeClass = traci.vehicle.getUniversal(constants.VAR_SHAPECLASS, vehID);