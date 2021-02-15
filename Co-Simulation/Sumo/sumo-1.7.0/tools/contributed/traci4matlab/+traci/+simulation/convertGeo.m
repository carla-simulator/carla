function [longitude latitude] = convertGeo(x, y, fromGeo)
%convertGeo Convert to geo-coordinates.
%   [LONGITUDE,LATITUDE] = convertGeo(X,Y)  Converts the position specified
%   by X and Y to geo-coordinates. Cartesian coordinates are assumed for
%   the input.
%   [X Y] = convert2D(...,FROMGEO) Specify if the conversion is to be
%   made from geo-coordinates, in this case, X is the longitude and Y is 
%   the latitude.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: convertGeo.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 3
    fromGeo = false;
end

fromType = sscanf(constants.POSITION_2D,'%x');
toType = sscanf(constants.POSITION_LON_LAT,'%x');

if fromGeo
     fromType = sscanf(constants.POSITION_LON_LAT,'%x');
     toType = sscanf(constants.POSITION_2D,'%x');
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.POSITION_CONVERSION,...
    '', 1+4 + 1+8+8 + 1+1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(2)];
message.string = [message.string uint8(fromType) ...
    traci.packInt64([y x])];
message.string = [message.string uint8([sscanf(constants.TYPE_UBYTE,'%x') toType])];
result = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.POSITION_CONVERSION, '');
longitude = result.readDouble;
latitude = result.readDouble;