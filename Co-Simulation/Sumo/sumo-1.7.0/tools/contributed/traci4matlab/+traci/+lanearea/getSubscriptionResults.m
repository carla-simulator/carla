function subscriptionResults = getSubscriptionResults(detID)
%getSubscriptionResults Get the subscription results for the last time step.
%   subscriptionResults = getSubscriptionResults(DETID) Returns the 
%   subscription results for the last time step and the given lanearea detector. If no 
%   lanearea detector id is given, all subscription results are returned in a 
%   containers.Map data structure.
%   If the lanearea detector id is unknown or the subscription did for any reason return
%   no data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSubscriptionResults.m 54 2019-01-03 15:41:54Z afacostag $

global laneareaSubscriptionResults
if isempty(laneareaSubscriptionResults)
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    detID = 'None';
end

subscriptionResults = laneareaSubscriptionResults.get(detID);