function changeTarget(vehID, edgeID)
%changeTarget Change the vehicle's destination.
%   changeTarget(VEHID,EDGEID) Changes the vehicle's destination edge to
%   the given. The route is rebuilt.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: changeTarget.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_CHANGETARGET, vehID, edgeID);