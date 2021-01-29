function emergencyStoppingIDList = getEmergencyStoppingVehiclesIDList()
%getEmergencyStoppingVehiclesIDList
%   emergencyStoppingIDList = getEmergencyStoppingVehiclesIDList()

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getEmergencyStoppingVehiclesIDList.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
emergencyStoppingIDList = traci.simulation.getUniversal(constants.VAR_EMERGENCYSTOPPING_VEHICLES_IDS);
