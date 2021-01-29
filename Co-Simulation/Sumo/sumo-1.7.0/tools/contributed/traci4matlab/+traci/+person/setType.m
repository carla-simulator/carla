function setType(personID, typeID)
%setType Sets the id of the type for the named person.
%   setType(PERSONID) Sets the id of the type for the named person.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setType.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendStringCmd(constants.CMD_SET_PERSON_VARIABLE, constants.VAR_TYPE,...
    personID, typeID);
