function remStages = getRemainingStages(personID)
%getRemainingStages Returns the number of remaining stages.
%   remStages = getRemainingStages(PERSONID) Returns the number of
%   remaining stages (at least 1).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getRemainingStages.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
remStages = traci.person.getUniversal(constants.VAR_STAGES_REMAINING, personID);
