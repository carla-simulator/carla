function nextStops = getNextStops(vehID)
%getNextStops List of upcoming stops.
%   nextStops = getNextStops(VEHID) Return list of upcoming stops [(lane,
%   endPos, stoppingPlaceID, stopFlags, duration, until), ...] where
%   integer stopFlag is defined as:
%       1 * stopped +
%       2 * parking +
%       4 * personTriggered +
%       8 * containerTriggered +
%       16 * isBusStop +
%       32 * isContainerStop +
%       64 * chargingStation +
%       128 * parkingarea
%   with each of these flags defined as 0 or 1.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getNextStops.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
nextStops = traci.vehicle.getUniversal(constants.VAR_NEXT_STOPS, vehID);
