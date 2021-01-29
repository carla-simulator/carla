function phase = getPhase(tlsID)
%getPhase Get the phase index.
%   phase = getPhase(TLSID) Returns the current phase index of
%   given trafficlights. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getPhase.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
phase = traci.trafficlights.getUniversal(constants.TL_CURRENT_PHASE, tlsID);