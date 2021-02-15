function stopEndingNumber = getStopEndingVehiclesNumber()
%getStopEndingVehiclesNumber
%   stopEndingNumber = getStopEndingVehiclesNumber()

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStopEndingVehiclesNumber.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
stopEndingNumber = traci.simulation.getUniversal(constants.VAR_STOP_ENDING_VEHICLES_NUMBER);
