function startingTeleportIDList = getStartingTeleportIDList()
%getStartingTeleportIDList Get the IDs of vehicles stsrting to teleport.
%   startingTeleportIDList = getStartingTeleportIDList() Returns a cell 
%   array of strings containing the ids of vehicles which which started to 
%   teleport in this time step.  

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getStartingTeleportIDList.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
startingTeleportIDList = traci.simulation.getUniversal(constants.VAR_TELEPORT_STARTING_VEHICLES_IDS);