function currentTime = getCurrentTime()
%getCurrentTime  Returns the current simulation time in ms.
%   currentTime = getCurrentTime() Returns the current simulation time in 
%   ms. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getCurrentTime.m 50 2018-12-28 16:25:47Z afacostag $

import traci.constants
warning('getCurrentTime is deprecated, please use getTime which returns floating point seconds')
currentTime = traci.simulation.getUniversal(constants.VAR_TIME_STEP);
