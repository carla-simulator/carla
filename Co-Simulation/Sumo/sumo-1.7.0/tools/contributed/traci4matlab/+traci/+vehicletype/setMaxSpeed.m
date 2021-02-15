function setMaxSpeed(typeID, speed)
%setMaxSpeed Sets the maximum speed for vehicles of this type.
%   setMaxSpeed(TYPEID,SPEED) Sets the maximum speed in m/s for vehicles of 
%   this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setMaxSpeed.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_MAXSPEED, typeID, speed);