function time = getTime()
%getTime  Returns the current simulation time in s.
%   time = getTime() Returns the current simulation time in s.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getTime.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
time = traci.simulation.getUniversal(constants.VAR_TIME);
