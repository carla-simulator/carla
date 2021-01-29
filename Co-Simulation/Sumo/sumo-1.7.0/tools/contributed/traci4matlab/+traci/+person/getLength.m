function length = getLength(personID)
%getLength Returns the length in m of the given person.
%   length = getLength(PERSONID) Returns the length in m of the given person.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLength.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
length = traci.person.getUniversal(constants.VAR_LENGTH, personID);