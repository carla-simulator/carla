function shapeClass = getShapeClass(typeID)
%getShapeClass Returns the shape class of vehicles of this type. 
%   shapeClass = getShapeClass(TYPEID) Returns the shape class of vehicles 
%   of this type. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getShapeClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
shapeClass = traci.vehicletype.getUniversal(constants.VAR_SHAPECLASS, typeID);