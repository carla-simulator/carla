function route = findRoute(fromEdge, toEdge, varargin)
%findRoute
%   route = findRoute(FROMEDGE,TOEDGE)
%   route = findRoute(...,VTYPE)
%   route = findRoute(...,DEPART)
%   route = findRoute(...,ROUTINGMODE)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: findRoute.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

vType = '';
depart = -1;
routingMode = 0;

if nargin > 2
    vType = varargin{1};
    if nargin > 3
        depart = varargin{2};
        if nargin > 4
            routingMode = varargin{3};
        end
    end
end

traci.beginMessage(constants.CMD_GET_SIM_VARIABLE, constants.FIND_ROUTE,...
    '', (1+4+1+4+length(fromEdge)) +1+4+length(toEdge) + 1+4+length(vType) + ...
    1+8+1+4);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(5)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(fromEdge)) uint8(fromEdge)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(toEdge)) uint8(toEdge)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(vType)) uint8(vType)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(depart) uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(routingMode)];
result = traci.checkResult(constants.CMD_GET_SIM_VARIABLE, constants.FIND_ROUTE, '');
route = traci.simulation.readStage(result);
