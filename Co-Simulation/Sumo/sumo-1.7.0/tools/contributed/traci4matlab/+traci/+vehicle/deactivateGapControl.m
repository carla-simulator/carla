function deactivateGapControl(vehID)
%deactivateGapControl Deactivate the vehicle's gap control.
%   deactivateGapControl(VEHID) Deactivate the vehicle's gap control.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: deactivateGapControl.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.vehicle.openGap(vehID, -1, -1, -1, -1);
