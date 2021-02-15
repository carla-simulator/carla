function lateralLanePosition = getLateralLanePosition(vehID)
%getAngle Get the lateral position of the vehicle.
%   lateralLanePosition = getLateralLanePosition(VEHID) Returns The lateral
%   position of the vehicle on its current lane measured in m. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLateralLanePosition.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lateralLanePosition = traci.vehicle.getUniversal(constants.VAR_LANEPOSITION_LAT, vehID);
