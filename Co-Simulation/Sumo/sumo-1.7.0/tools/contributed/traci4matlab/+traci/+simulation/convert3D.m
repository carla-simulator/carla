function [x,y,z] = convert3D(edgeID, pos, varargin)
%convert3D Convert to absolute coordinates from those relative to an edge.
%   [X,Y,Z] = convert3D(EDGEID,POS) Converts from the position POS relative
%   to the edge EDGEID to absolute coordinates.
%   [X,Y,Z] = convert2D(...,LANEINDEX) Specify the index of the edge's lane.
%   [X,Y,Z] = convert2D(...,TOGEO) Specify if the conversion is made to
%   geo-coordinates, in this case, X is the longitude and Y is the latitude.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: convert3D.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

laneIndex = 0;
toGeo = false;

if nargin > 2
    laneIndex = varargin{1};
    if nargin > 3
        toGeo = varargin{2};
    end
end

posType = sscanf(constants.POSITION_3D,'%x');

if toGeo
    posType = sscanf(constants.POSITION_LON_LAT_ALT,'%x');
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.POSITION_CONVERSION,...
    '', 1+4 + 1+4+length(edgeID)+8+1 + 1+1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2)];
message.string = [message.string uint8(sscanf(constants.POSITION_ROADMAP,'%x')) ...
    traci.packInt32(length(edgeID)) uint8(edgeID)];
message.string = [message.string traci.packInt64(pos) ...
    uint8([laneIndex sscanf(constants.TYPE_UBYTE,'%x') posType])];
result = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.POSITION_CONVERSION, '');
x = result.readDouble;
y = result.readDouble;
z = result.readDouble;
