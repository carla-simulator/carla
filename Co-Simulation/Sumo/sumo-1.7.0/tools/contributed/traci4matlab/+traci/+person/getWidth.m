function width = getWidth(personID)
%getWidth Returns the width in m of this person.
%   width = getWidth(PERSONID) Returns the width in m of this person.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getWidth.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
width = traci.person.getUniversal(constants.VAR_WIDTH, personID);