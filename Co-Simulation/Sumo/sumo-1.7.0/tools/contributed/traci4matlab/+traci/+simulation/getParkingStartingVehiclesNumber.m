function parkingStartingNumber = getParkingStartingVehiclesNumber()
%getParkingStartingVehiclesNumber
%   parkingStartingNumber = getParkingStartingVehiclesNumber()

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getParkingStartingVehiclesNumber.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
parkingStartingNumber = traci.simulation.getUniversal(constants.VAR_PARKING_STARTING_VEHICLES_NUMBER);
