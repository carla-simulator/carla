function setAccel(typeID, accel)
%setAccel Sets the maximum acceleration for vehicles of this type.
%   setAccel(TYPEID,ACCEL) Sets the maximum acceleration in m/s^2 for 
%   vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setAccel.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_ACCEL, typeID, accel);