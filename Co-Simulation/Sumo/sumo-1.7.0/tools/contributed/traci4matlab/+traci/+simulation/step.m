function step(varargin)
%step Make a simulation step.
%   step() Perform exactly one simulation step.
%   step(TIME) simulate up to the given sim time (in seconds).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: step.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

time = 0;
if nargin > 0
    time = varargin{1};
end

traci.simulationStep(time);
