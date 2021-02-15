function program = getProgram(tlsID)
%getProgram Returns the id of the current program.
%   program = getProgram(TLSID) Returns the id of the current program. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getProgram.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
program = traci.trafficlights.getUniversal(constants.TL_CURRENT_PROGRAM, tlsID);