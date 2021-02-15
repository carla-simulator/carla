function setColor(personID, color)
%setColor Sets the color of this person.
%   setColor(PERSONID) Sets the color for the vehicle with the given ID,
%   i.e. (255,0,0) for the color red. The fourth component (alpha) is
%   optional.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setColor.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if length(color) > 3
    alpha = color(4);
else
    alpha = 255;
end

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE, constants.VAR_COLOR,...
    personID, 1+1+1+1+1);
message.string = [message.string uint8([sscanf(constants.TYPE_COLOR,'%x') ...
    int32(color(1)) int32(color(2)) int32(color(3)) alpha])];
traci.sendExact();
