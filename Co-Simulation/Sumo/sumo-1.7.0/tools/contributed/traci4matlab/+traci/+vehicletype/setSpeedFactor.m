function setSpeedFactor(typeID, factor)
%setSpeedFactor Sets the speed factor for vehicles of this type.
%   setSpeedFactor(TYPEID,FACTOR) Sets the speed factor for vehicles of
%   this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setSpeedFactor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_SPEED_FACTOR, typeID, factor);