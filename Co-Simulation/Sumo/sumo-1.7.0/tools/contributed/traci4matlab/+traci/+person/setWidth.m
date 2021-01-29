function setWidth(personID, width)
%setWidth Sets the width in m for this person.
%   setWidth(PERSONID) Sets the width in m for this person.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setWidth.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendDoubleCmd(constants.CMD_SET_PERSON_VARIABLE, constants.VAR_WIDTH,...
    personID, width);
