function noiseEmission = getNoiseEmission(vehID)
%getNoiseEmission Get the noise emission of the vehicle.
%   noiseEmission = getNoiseEmission(VEHID) Returns the noise emission in 
%   db for the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getNoiseEmission.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
noiseEmission = traci.vehicle.getUniversal(constants.VAR_NOISEEMISSION, vehID);