function [roadID pos laneID] = convertRoad(x, y, isGeo)
%convertRoad Convert to coordinates relative to a road from absolute coordinates.
%   [ROADID POS LANEID] = convertRoad(X,Y) Converts from the absolute
%   position defined by X and Y, to a position POS relative to the road 
%   ROADID and the lane LANEID. Cartesian coordinates are assumed for the
%   input.
%   [ROADID POS LANEID] = convertRoad(...,ISGEO) Specify if the absolute
%   input position is in geo-coordinates.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: convertRoad.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 3
    isGeo = false;
end

posType = sscanf(constants.POSITION_2D,'%x');

if isGeo
    posType = sscanf(constants.POSITION_LON_LAT,'%x');
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.POSITION_CONVERSION,...
    '', 1+4 + 1+8+8 + 1+1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2)];
message.string = [message.string uint8(posType) ...
    traci.packInt64([y x])];
message.string = [message.string uint8([sscanf(constants.TYPE_UBYTE,'%x') ...
    sscanf(constants.POSITION_ROADMAP,'%x')])];
result = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.POSITION_CONVERSION, '');
roadID = result.readString();
pos = result.readDouble();
laneID = result.read(1);