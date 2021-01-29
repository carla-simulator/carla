function phaseDuration = getPhaseDuration(tlsID)
%getPhaseDuration Get the duration of the traffic lights' current phase.
%   phaseDuration = getPhaseDuration(TLSID) Returns the phase duration of 
%   the current phase of the named traffic lights.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPhaseDuration.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
phaseDuration = traci.trafficlights.getUniversal(constants.TL_PHASE_DURATION, tlsID);
