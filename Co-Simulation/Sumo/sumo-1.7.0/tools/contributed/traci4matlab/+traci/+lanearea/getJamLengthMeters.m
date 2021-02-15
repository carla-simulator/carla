function JamLengthMeters = getJamLengthMeters(detID)
%getJamLengthMeters Return the jam length in vehicles.
%   JamLengthVehicle = getJamLengthVehicle(DETID) Returns the
%	jam length in meters within the last simulation step on
%	the given lanearea detector.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getJamLengthMeters.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
JamLengthMeters = traci.lanearea.getUniversal(constants.JAM_LENGTH_METERS, detID);