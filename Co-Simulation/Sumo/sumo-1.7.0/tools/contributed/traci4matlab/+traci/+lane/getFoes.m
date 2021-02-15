function foes = getFoes(laneID, toLaneID)
%getFoes Returns lanes that have right of way.
%   foes = getFoes(LANEID,TOLANEID) Returns the ids of incoming lanes that
%   have right of way over the connection from LANE to TOLANEID.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getFoes.m 54 2019-01-03 15:41:54Z afacostag $

global message
import traci.constants
traci.beginMessage(constants.CMD_GET_LANE_VARIABLE, constants.VAR_FOES,...
    laneID, 1 + 4 + length(toLaneID));
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x'))...
    traci.packInt32(length(toLaneID)) uint8(toLaneID)];
result = traci.checkResult(constants.CMD_GET_LANE_VARIABLE,...
    constants.VAR_FOES, laneID);
foes = result.readStringList();
