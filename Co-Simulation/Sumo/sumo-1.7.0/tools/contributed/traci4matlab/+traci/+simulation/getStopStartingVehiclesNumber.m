function stopStartingNumber = getStopStartingVehiclesNumber()
%getStopStartingVehiclesNumber
%   stopStartingNumber = getStopStartingVehiclesNumber()

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStopStartingVehiclesNumber.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
stopStartingNumber = traci.simulation.getUniversal(constants.VAR_STOP_STARTING_VEHICLES_NUMBER);
