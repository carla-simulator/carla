function setMinGap(typeID, minGap)
%setMinGap Sets the offset for vehicles of this type.
%   setMinGap(TYPEID,MINGAP) Sets the offset (gap to front vehicle if 
%   halting) for vehicles of this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setMinGap.m 50 2018-12-28 16:25:47Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_MINGAP, typeID, minGap);
