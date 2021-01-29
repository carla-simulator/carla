function stopEndingIDList = getStopEndingVehiclesIDList()
%getStopEndingVehiclesIDList
%   stopEndingIDList = getStopEndingVehiclesIDList()

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStopEndingVehiclesIDList.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
stopEndingIDList = traci.simulation.getUniversal(constants.VAR_STOP_ENDING_VEHICLES_IDS);
