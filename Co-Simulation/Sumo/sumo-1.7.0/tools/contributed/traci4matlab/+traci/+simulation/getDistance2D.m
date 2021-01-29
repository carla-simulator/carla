function distance2D = getDistance2D(x1, y1, x2, y2, isGeo, isDriving)
%getDistance2D Get the distance between two coordinates.
%   distance2D = getDistance2D(X1,Y1,X2,Y2) Returns the distance between
%   the points defined by the 2D coordinates (X1,Y1) and (X2,Y2).
%   Cartesian coordinates are assumed.
%   distance2D = getDistance2D(...,ISGEO) Specify wether the inputs are 
%   in geo-coordinates.
%   distance2D = getDistance2D(...,ISDRIVING) Specify wether the air or the
%   driving distance shall be computed.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getDistance2D.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message
if nargin < 6
    isDriving = false;
    if nargin < 5
        isGeo = false;
    end
end

posType = sscanf(constants.POSITION_2D,'%x');

if isGeo
    posType = sscanf(constants.POSITION_LON_LAT,'%x');
end

distType = sscanf(constants.REQUEST_AIRDIST,'%x');

if isDriving
    distType = sscanf(constants.REQUEST_DRIVINGDIST,'%x');
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.DISTANCE_REQUEST,...
    '', 1+4 + 1+8+8 + 1+8+8 + 1);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(3)];
message.string = [message.string uint8(posType) ...
    traci.packInt64([y1 x1])];
message.string = [message.string uint8(posType) ...
    traci.packInt64([y2 x2]) uint8(distType)];
result = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.DISTANCE_REQUEST, '');
distance2D = result.readDouble();