function nextSwitch = getNextSwitch(tlsID)
%getNextSwitch Returns the time in ms at which the next phase change will
%be performed.
%   nextSwitch = getNextSwitch(TLSID) Returns the time in ms at which the
%   next phase change will be performed. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getNextSwitch.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
nextSwitch = traci.trafficlights.getUniversal(constants.TL_NEXT_SWITCH, tlsID);