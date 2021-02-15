function rerouteTraveltime(personID)
%rerouteTraveltime Reroutes a pedestrian (walking person).
%   rerouteTraveltime(PERSONID) Reroutes a pedestrian (walking person).

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: rerouteTraveltime.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE,...
    constants.CMD_REROUTE_TRAVELTIME, personID, 1+4);

message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(0)];
traci.sendExact();
