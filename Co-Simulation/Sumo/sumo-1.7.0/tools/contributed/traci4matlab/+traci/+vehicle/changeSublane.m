function changeSublane(vehID, latDist)
%changeSublane Forces a lateral change by the given amount.
%   changeSublane(VEHID,LATDIST) Forces a lateral change by the given
%   amount (negative values indicate changing to the right, positive to the
%   left). This will override any other lane change motivations but conform
%   to safety-constraints as configured by laneChangeMode.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: changeSublane.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.CMD_CHANGESUBLANE, vehID, latDist);
