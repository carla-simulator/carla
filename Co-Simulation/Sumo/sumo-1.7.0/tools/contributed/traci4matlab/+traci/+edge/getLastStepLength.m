function lastStepLength = getLastStepLength(edgeID)
%getLastStepLength Get the mean vehicle length on the edge.
%   lastStepLength = getLastStepLength(EDGEID) Returns the mean vehicle 
%   length in m for the last time step on the given edge.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getLastStepLength.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
lastStepLength = traci.edge.getUniversal(constants.LAST_STEP_LENGTH, edgeID);