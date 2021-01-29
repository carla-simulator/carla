function screenshot(viewID, filename, width, height)
%screenshot Save a screenshot of the SUMO gui.
%   screenshot(VIEWID, FILENAME) Save a screenshot for the given view to 
%   the given filename. The fileformat is guessed from the extension, the 
%   available formats differ from platform to platform but should at least
%   include ps, svg and pdf, on linux probably gif, png and jpg as well.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: screenshot.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message

traci.beginMessage(constants.CMD_SET_GUI_VARIABLE, constants.VAR_SCREENSHOT, viewID,...
    1+4+1+4+length(filename)+1+4+1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(3)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(filename)) uint8(filename)];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(width)];
message.string = [message.string uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(height)];
traci.sendExact();