function roadID = getRoadID(personID)
%getRoadID Returns the edge the person was at in last step. 
%   roadID = getRoadID(PERSONID) Returns the id of the edge the named person 
%   was at within the last step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getRoadID.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
roadID = traci.person.getUniversal(constants.VAR_ROAD_ID, personID);