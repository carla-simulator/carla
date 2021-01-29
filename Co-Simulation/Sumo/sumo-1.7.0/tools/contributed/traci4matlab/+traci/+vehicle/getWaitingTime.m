function waitingTime = getWaitingTime(vehID)
%getWaitingTime Get the waiting time of the vehicle.
%   waitingTime = getWaitingTime(VEHID) The waiting time of a vehicle is
%   defined as the time (in seconds) spent with a speed below 0.1m/s since
%   the last time it was faster than 0.1m/s. (basically, the waiting time
%   of a vehicle is reset to 0 every time it moves). A vehicle that is
%   stopping intentionally with a <stop> does not accumulate waiting time. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getWaitingTime.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
waitingTime = traci.vehicle.getUniversal(constants.VAR_WAITING_TIME, vehID);
