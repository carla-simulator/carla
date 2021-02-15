function setLength(personID, length)
%setLength Sets the length in m for this person.
%   setLength(PERSONID) Sets the length in m for this person.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setLength.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendDoubleCmd(constants.CMD_SET_PERSON_VARIABLE, constants.VAR_LENGTH,...
    personID, length);
