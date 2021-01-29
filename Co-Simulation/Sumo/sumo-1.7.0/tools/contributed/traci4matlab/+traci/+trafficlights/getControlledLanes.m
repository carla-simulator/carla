function controlledLanes = getControlledLanes(tlsID)
%getControlledLanes Get the controlled lanes.
%   controlledLanes = getControlledLanes(TLSID) Returns a cell array of 
%   strings containing the ids of the lanes controlled by the given traffic
%   lights. 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getControlledLanes.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
controlledLanes = traci.trafficlights.getUniversal(constants.TL_CONTROLLED_LANES, tlsID);