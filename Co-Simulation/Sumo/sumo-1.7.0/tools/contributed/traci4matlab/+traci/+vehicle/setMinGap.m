function setMinGap(vehID, minGap)
%setMinGap Sets the offset (gap to front vehicle) for this vehicle.
%   setMinGap(VEHID,MINGAP) Sets the offset (gap to front vehicle if 
%   halting) for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setMinGap.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_MINGAP, vehID, minGap);
