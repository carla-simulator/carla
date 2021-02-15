function vehicle = getVehicle(personID)
%getVehicle Returns the id of the current vehicle.
%   vehicle = getVehicle(PERSONID) Returns the id of the current vehicle if
%   the person is in stage driving and has entered a vehicle. Return the
%   empty string otherwise

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getVehicle.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
vehicle = traci.person.getUniversal(constants.VAR_VEHICLE, personID);
