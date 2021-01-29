function subscriptionResults = getSubscriptionResults(laneID)
%getSubscriptionResults Get the subscription results for the last time step.
%   subscriptionResults = getSubscriptionResults(LANEID) Returns the 
%   subscription results for the last time step and the given lane. If no 
%   lane id is given, all subscription results are returned in a 
%   containers.Map data structure.
%   If the lane id is unknown or the subscription did for any reason return
%   no data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSubscriptionResults.m 48 2018-12-26 15:35:20Z afacostag $

global laneSubscriptionResults
if isempty(laneSubscriptionResults)
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    laneID = 'None';
end

subscriptionResults = laneSubscriptionResults.get(laneID);