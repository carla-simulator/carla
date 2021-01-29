function setOffset(viewID, x, y)
%setOffset Set the offset of the view.
%   setOffset(VIEWID, X, Y) Set the current offset for the given view.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setOffset.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_SET_GUI_VARIABLE, constants.VAR_VIEW_OFFSET, viewID, 1+8+8);
message.string = [message.string uint8(sscanf(constants.POSITION_2D,'%x'))...
    traci.packInt64([y x])];
traci.sendExact();