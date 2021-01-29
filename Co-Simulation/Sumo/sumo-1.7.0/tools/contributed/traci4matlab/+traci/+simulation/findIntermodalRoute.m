function result = findIntermodalRoute(fromEdge, toEdge, varargin)
%findIntermodalRoute
%   route = findIntermodalRoute(FROMEDGE,TOEDGE)
%   route = findIntermodalRoute(...,MODES)
%   route = findRoute(...,DEPART)
%   route = findRoute(...,ROUTINGMODE)
%   route = findRoute(...,SPEED)
%   route = findRoute(...,WALKFACTOR)
%   route = findRoute(...,DEPARTPOS)
%   route = findRoute(...,ARRIVALPOS)
%   route = findRoute(...,DEPARTPOSLAT)
%   route = findRoute(...,PTYPE)
%   route = findRoute(...,VTYPE)
%   route = findRoute(...,DESTSTOP)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: findIntermodalRoute.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

modes = '';
depart = -1;
routingMode = 0;
speed = -1;
walkFactor = -1;
departPos = 0;
arrivalPos = constants.INVALID_DOUBLE_VALUE;
departPosLat = 0;
pType = '';
vType = '';
destStop = '';

if nargin > 2
    modes = varargin{1};
    if nargin > 3
        depart = varargin{2};
        if nargin > 4
            routingMode = varargin{3};
            if nargin > 5
                speed = varargin{4};
                if nargin > 6
                    walkFactor = varargin{5};
                    if nargin > 7
                        departPos = varargin{6};
                        if nargin > 8
                            arrivalPos = varargin{7};
                            if nargin > 9
                                departPosLat = varargin{8};
                                if nargin > 10
                                    pType = varargin{9};
                                    if nargin > 11
                                        vType = varargin{10};
                                        if nargin > 12
                                            destStop = varargin{11};
                                        end
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
    end
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.FIND_INTERMODAL_ROUTE,...
    '', (1+4+1+4+length(fromEdge)) +1+4+length(toEdge) + 1+4+length(modes) + ...
    1+8+1+4+1+8+1+8+1+8+1+8+1+8+1+4+length(pType)+1+4+length(vType)+1+4+ ...
    length(destStop));
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(13)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(fromEdge)) uint8(fromEdge)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(toEdge)) uint8(toEdge)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(modes)) uint8(modes)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(depart) uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(routingMode)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(speed) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(walkFactor)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(departPos) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(arrivalPos)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(departPosLat)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(pType)) uint8(pType)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(vType)) uint8(vType)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(destStop)) uint8(destStop)];

answer = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.FIND_INTERMODAL_ROUTE, '');
answerLength = answer.readInt();
result = cell(1, answerLength);

for i = 1:answerLength
    answer.read(1);
    result{i} = traci.simulation.readStage(answer);
end
