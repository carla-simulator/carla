function addSubscriptionFilter(filterType, params)
%addSubscriptionFilter An internal function to add subscription filters.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id$

import traci.constants
global message

if nargin < 2
    params = [];
end

command = constants.CMD_ADD_SUBSCRIPTION_FILTER;
message.queue = [message.queue uint8(sscanf(command,'%x'))];

if ismember(filterType, {constants.FILTER_TYPE_NONE,...
        constants.FILTER_TYPE_NOOPPOSITE,constants.FILTER_TYPE_TURN,...
        constants.FILTER_TYPE_LEAD_FOLLOW})
    assert(isempty(params));
    len = 1+1+1;
    message.string = [message.string uint8([len,sscanf(command,'%x'),...
        sscanf(filterType,'%x')])];
elseif ismember(filterType,{constants.FILTER_TYPE_DOWNSTREAM_DIST,...
        constants.FILTER_TYPE_UPSTREAM_DIST})
    assert(isnumeric(params))
    len = 1+1+1+1+8;
    message.string = [message.string uint8([len, sscanf(command,'%x'),...
        sscanf(filterType,'%x'),sscanf(constants.TYPE_DOUBLE,'%x')]) ...
        traci.packInt64(params)];
elseif ismember(filterType, {constants.FILTER_TYPE_VCLASS,...
        constants.FILTER_TYPE_VTYPE})
    len = 1+1+1+1+4;
    try
        for i = 1:length(params)
            len = len + 4 + length(params{i});
        end
    catch e
        throw(MException('TraCI4Matlab:InvalidArgument',...
            sprintf('Filter type %s requires identifier list as parameter.',...
            filterType)))
    end
    
    if len <= 255
        message.string = [message.string uint8([len sscanf(command,'%x') ...
            sscanf(filterType,'%x')])];
    else
        len = len + 4;
        message.string = [message.string uint8(0) traci.packInt32(len) ...
            uint8([sscanf(command,'%x') sscanf(filterType,'%x')])];
    end
    message.string = [message.string uint8(sscanf(...
        constants.TYPE_STRINGLIST,'%x')) traci.packInt32(length(params))];
    for i = 1:length(params)
        message.string = [message.string traci.packInt32(length(params{1})) ...
            uint8(params{i})];
    end
elseif strcmp(filterType, constants.FILTER_TYPE_LANES)
    lanes = unique(params);
    if length(lanes) ~= length(params)
        warning('Ignoring duplicate lane specification for subscription filter.');
    end
    len = 1+1+1+1+length(lanes);
    message.string = [message.string uint8([len sscanf(command,'%x') ...
        sscanf(filterType,'%x'), length(lanes)])];
    for i = 1:length(lanes)
        if lanes(i) <= -128 && lanes{i} >= 128
            throw(MException('TraCI4Matlab:InvalidArgument',...
                'Filter type lanes requires numeric index list as parameter.'));
        end
        if lanes(i) < 0
            lanes(i) = lanes(i) + 256;
        end
        message.string = [message.string uint8(lanes(i))];
    end
end
