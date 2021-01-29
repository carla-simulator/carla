function add(poiID, x, y, color, poiType, layer)
%add Add a point of interest (poi) to the SUMO network.
%   add(POIID,X,Y,COLOR) Adds a poi with the id POIID to the coordinates
%   X and Y, with the specified color. COLOR is a four-element vector whose
%   elements range from 0 to 255, they represent the R, G, B and Alpha 
%   (unused) components of the color.
%   add(...,POITYPE) Assigns a type POITYPE to the added poi.
%   add(...,LAYER) Adds the poi to the given layer.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: add.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global message

if nargin < 6
    layer = 0;
    if nargin < 5
        poiType = '';
    end
end
typeCompound = sscanf(constants.TYPE_COMPOUND,'%x');
typeString = sscanf(constants.TYPE_STRING,'%x');
typeColor = sscanf(constants.TYPE_COLOR,'%x');
typeInteger = sscanf(constants.TYPE_INTEGER,'%x');
position2D = sscanf(constants.POSITION_2D,'%x');
traci.beginMessage(constants.CMD_SET_POI_VARIABLE, constants.ADD, poiID,...
 1+4 + 1+4+length(poiType) + 1+1+1+1+1 + 1+4 + 1+8+8);
message.string = [message.string uint8(typeCompound) traci.packInt32(4)];
message.string = [message.string uint8(typeString) traci.packInt32(length(poiType)) uint8(poiType)];
message.string = [message.string uint8([typeColor color])];
message.string = [message.string uint8(typeInteger) traci.packInt32(layer)];
message.string = [message.string uint8(position2D) traci.packInt64([x y])];
traci.sendExact();