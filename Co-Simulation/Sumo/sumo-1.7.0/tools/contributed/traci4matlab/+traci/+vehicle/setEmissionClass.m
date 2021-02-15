function setEmissionClass(vehID, clazz)
%setEmissionClass Sets the emission class for this vehicle.
%   setEmissionClass(VEHID,CLASS) Sets the emission class for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setEmissionClass.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_EMISSIONCLASS, vehID, clazz);