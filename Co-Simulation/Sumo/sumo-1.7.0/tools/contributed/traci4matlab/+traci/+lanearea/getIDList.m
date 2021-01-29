function IDList = getIDList()
%getIDList Get the IDs of the lanearea detectors in the network.
%   IDList = getIDList() Returns a cell array of strings containing the IDs
%   of the lanearea detectors in the SUMO network.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getIDList.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
IDList = traci.lanearea.getUniversal(constants.ID_LIST, '');