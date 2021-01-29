function removeStages(personID)
%removeStages Removes all stages of the person.
%   removeStages(PERSONID) Removes all stages of the person. If no new
%   phases are appended, the person will be removed from the simulation in
%   the next simulationStep().

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: removeStages.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

while traci.person.getRemainingStages(personID) > 1
    traci.person.removeStage(personID, 1)
end

traci.person.removeStage(personID, 0);
