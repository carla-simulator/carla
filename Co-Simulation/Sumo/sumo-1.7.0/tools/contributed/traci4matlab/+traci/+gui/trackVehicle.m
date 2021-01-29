function trackVehicle(viewID, vehID)
%trackVehicle Track vehicle in SUMO gui.
%   trackVehicle(viewID, vehID) Start visually tracking the given vehicle 
%   on the given view.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: trackVehicle.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_GUI_VARIABLE, constants.VAR_TRACK_VEHICLE, viewID, vehID);