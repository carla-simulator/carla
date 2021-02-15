function setColor(vehID, color)
%setColor Set the color of the vehicle.
%   setColor(VEHID,COLOR) Sets color for vehicle with the given ID, i.e. 
%   (255,0,0,0) for the color red. The fourth integer (alpha) is currently 
%   ignored

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setColor.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_COLOR, vehID, 1+1+1+1+1);
message.string = [message.string uint8([sscanf(constants.TYPE_COLOR,'%x') color])];
traci.sendExact();