function setBoundary(viewID, xmin, ymin, xmax, ymax)
%setBoundary Set the coordinates of the view.
%   setBoundary(VIEWID, XMIN, YMIN, XMAX, YMAX) Set the current boundary 
%   for the given view (see getBoundary).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setBoundary.m 53 2019-01-03 15:18:31Z afacostag $

import traci.constants
global message
traci.beginMessage(constants.CMD_SET_GUI_VARIABLE,...
    constants.VAR_VIEW_BOUNDARY, viewID, 1+1+8+8+8+8);
message.string = [message.string uint8(sscanf(constants.TYPE_POLYGON,'%x'))...
    uint8(2) traci.packInt64([xmin ymin xmax ymax])];
traci.sendExact();