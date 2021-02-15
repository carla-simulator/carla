function personIDs = getPersonIDList(vehID)
%getPersonIDList Get a cell array with the IDs of persons in the vehicle.
%   personIDs = getPersonIDList(VEHID) Returns the list of persons which
%   includes those defined using attribute 'personNumber' as well as
%   <person>-objects which are riding in this vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPersonIDList.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
personIDs = traci.vehicle.getUniversal(constants.LAST_STEP_PERSON_ID_LIST, vehID);
