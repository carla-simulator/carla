function add(personID, edgeID, pos, depart, typeID)
%add Inserts a new person to the simulation.
%   add(PERSONID,EDGEID,POS) Inserts a new person to the simulation at the
%   given edge, position. This function should be followed by appending
%   Stages or the person.
%   add(...,DEPART) Specify the depart time in seconds.
%   add(...,TYPEID) Specify the person type ID.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: add.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if nargin < 5
    typeID = 'DEFAULT_PEDTYPE';
    if nargin < 4
        depart = sscanf(constants.DEPARTFLAG_NOW,'%x');
    end
end

traci.beginMessage(constants.CMD_SET_PERSON_VARIABLE, constants.ADD,...
    personID, 1+4+1+4+length(typeID)+1+4+length(edgeID)+1+8+1+8);

message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(4)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(typeID)) uint8(typeID)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(edgeID)) uint8(edgeID)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(depart)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(pos)];
traci.sendExact();
