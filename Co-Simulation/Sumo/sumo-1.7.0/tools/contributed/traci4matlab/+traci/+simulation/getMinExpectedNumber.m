function minExpectedNumber = getMinExpectedNumber()
%getMinExpectedNumber Get the number of non-arrived vehicles.
%   minExpectedNumber = getMinExpectedNumber() Returns the number of 
%   vehicles which are in the net plus the ones still waiting to start. 
%   This number may be smaller than the actual number of vehicles still to 
%   come because of delayed route file parsing. If the number is 0 however,
%   it is guaranteed that all route files have been parsed completely and
%   all vehicles have left the network. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getMinExpectedNumber.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
minExpectedNumber = traci.simulation.getUniversal(...
    constants.VAR_MIN_EXPECTED_VEHICLES);