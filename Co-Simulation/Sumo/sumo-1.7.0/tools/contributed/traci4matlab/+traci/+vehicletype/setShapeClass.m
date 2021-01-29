function setShapeClass(typeID, clazz)
%setShapeClass Sets the shape class for vehicles of this type. 
%   setShapeClass(TYPEID,CLASS) Sets the shape class for vehicles of this 
%   type. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setShapeClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_SHAPECLASS, typeID, clazz);