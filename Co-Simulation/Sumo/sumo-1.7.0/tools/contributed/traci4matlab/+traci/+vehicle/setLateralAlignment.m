function setLateralAlignment(vehID, align)
%setLateralAlignment Sets the lateral alignment.
%   setLateralAlignment(VEHID,ALIGN) Sets the preferred lateral alignment
%   for this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setLateralAlignment.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_LATALIGNMENT, vehID, align);
