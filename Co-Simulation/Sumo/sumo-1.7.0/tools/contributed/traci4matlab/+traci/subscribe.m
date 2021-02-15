function subscribe(cmdID, subscriptionBegin, subscriptionEnd, objID, varIDs, varargin)
%subscribe An internal function to build a subscription message and parse
%the response.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: subscribe.m 53 2019-01-03 15:18:31Z afacostag $

global message

parameters = containers.Map();

if nargin > 5
    parameters = varargin{1};
end

% Construct the TraCI message
message.queue = [message.queue uint8(sscanf(cmdID,'%x'))];
len = 1+1+8+8+4+length(objID)+1+length(varIDs);

if ~isempty(parameters)
    for i = 1:length(varIDs)
        if parameters.isKey(varIDs{i})
            len = len + parameters(varIDs{i});
        end
    end
end

if len <= 255
    message.string = [message.string uint8(len)];
else
    message.string = [message.string uint8(0) traci.packInt32(len+4)];
end

message.string = [message.string uint8(sscanf(cmdID,'%x')),...
    traci.packInt64([subscriptionBegin subscriptionEnd]) ...
    traci.packInt32(length(objID)) uint8(objID)];
message.string = [message.string uint8(length(varIDs))];
for v=1:length(varIDs)
    message.string = [message.string uint8(sscanf(varIDs{v},'%x'))];
    if ~isempty(parameters) && parameters.isKey(varIDs{v})
        message.string = [message.string uint8(sscanf(varIDs{v},'%x'))];
    end
end

% Send the TraCI message and receive the result
result = traci.sendExact();

% Populate the subsctiptions and parse the result
[objectID,response] = traci.readSubscription(result);
if response - uint8(sscanf(cmdID,'%x'))~=16 || ~strcmp(objectID,objID) 
    raise(MException('traci:FatalTraciError',['Received answer ' response ...
        ', ' objectID 'for subscription command ' cmdID ', ' objID]));
end
