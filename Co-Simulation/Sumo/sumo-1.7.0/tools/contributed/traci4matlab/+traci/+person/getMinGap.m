function minGap = getMinGap(personID)
%getMinGap Returns the gap to front person.
%   minGap = getMinGap(PERSONID) Returns the offset (gap to front person if 
%   halting) of this person.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getMinGap.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
minGap = traci.person.getUniversal(constants.VAR_MINGAP, personID);