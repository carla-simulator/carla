function waitingTime = getWaitingTime(personID)
%getWaitingTime Returns the waiting time of the given person.
%   waitingTime = getWaitingTime(PERSONID) The waiting time of a person is
%   defined as the time (in seconds) spent with a speed below 0.1m/s since
%   the last time it was faster than 0.1m/s. (basically, the waiting time
%   of a person is reset to 0 every time it moves).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getWaitingTime.m 48 2018-12-26 15:35:20Z afacostag $  

import traci.constants
waitingTime = traci.person.getUniversal(constants.VAR_WAITING_TIME, personID);