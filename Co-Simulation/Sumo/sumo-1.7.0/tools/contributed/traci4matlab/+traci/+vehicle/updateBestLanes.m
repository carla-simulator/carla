function updateBestLanes(vehID)
%updateBestLanes Triggers an update of the vehicle's bestLanes.
%   updateBestLanes(VEHID) Triggers an update of the vehicle's bestLanes
%   (structure determining the lane preferences used by LC models) It may
%   be called after modifying the vClass for instance.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: updateBestLanes.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_UPDATE_BESTLANES, vehID);
