function color = getColor(personID)
%getColor Get the color of the person.
%   color = getColor(PERSONID) Returns the person's rgba color.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getColor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
color = traci.person.getUniversal(constants.VAR_COLOR, personID);