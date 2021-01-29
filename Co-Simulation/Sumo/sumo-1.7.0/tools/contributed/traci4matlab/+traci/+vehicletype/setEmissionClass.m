function setEmissionClass(typeID, clazz)
%setEmissionClass ets the emission class of vehicles for this type.
%   setEmissionClass(TYPEID,CLASS) Sets the emission class of vehicles of 
%   this type.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setEmissionClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE, constants.VAR_EMISSIONCLASS, typeID, clazz);