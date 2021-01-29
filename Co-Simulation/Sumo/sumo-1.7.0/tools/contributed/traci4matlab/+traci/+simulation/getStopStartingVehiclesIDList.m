function stopStartingIDList = getStopStartingVehiclesIDList()
%getStopStartingVehiclesIDList
%   stopStartingIDList = getStopStartingVehiclesIDList()

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStopStartingVehiclesIDList.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
stopStartingIDList = traci.simulation.getUniversal(constants.VAR_STOP_STARTING_VEHICLES_IDS);
