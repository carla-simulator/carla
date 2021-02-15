function setHeight(typeID, height)
%setHeight Sets the height in m for vehicles of this type.
%   setHeight(TYPEID,HEIGHT) Sets the height in m of vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setHeight.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_HEIGHT, typeID, height);
