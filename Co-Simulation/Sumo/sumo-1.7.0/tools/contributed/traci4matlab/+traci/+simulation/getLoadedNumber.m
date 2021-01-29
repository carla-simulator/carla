function loadedNumber = getLoadedNumber()
%getLoadedNumber Get the number loaded vehicles.
%   loadedNumber = getLoadedNumber() Returns the number of vehicles which 
%   were loaded in this time step. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLoadedNumber.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
loadedNumber = traci.simulation.getUniversal(constants.VAR_LOADED_VEHICLES_NUMBER);