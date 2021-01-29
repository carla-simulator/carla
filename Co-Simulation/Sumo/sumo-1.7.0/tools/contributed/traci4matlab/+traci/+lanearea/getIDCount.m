function IDCount = getIDCount()
%getIDCount Get the number of aeral detectors in the SUMO network.  

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getIDCount.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
IDCount = traci.lanearea.getUniversal(constants.ID_COUNT, '');