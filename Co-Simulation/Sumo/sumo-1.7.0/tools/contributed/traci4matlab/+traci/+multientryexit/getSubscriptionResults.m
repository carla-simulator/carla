function subscriptionResults = getSubscriptionResults(detID)
%getSubscriptionResults Get the subscription results for the last time step.
%   subscriptionResults = getSubscriptionResults(DETID) Returns the 
%   subscription results for the last time step and the given 
%   multi-entry/multi-exit detector. If no multi-entry/multi-exit detector 
%   id is given, all subscription results are returned in a containers.Map 
%   data structure.
%   If the multi-entry/multi-exit detector id is unknown or the subscription did for any reason return
%   no data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSubscriptionResults.m 48 2018-12-26 15:35:20Z afacostag $

global detSubscriptionResults
if isempty(detSubscriptionResults)
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    detID = 'None';
end

subscriptionResults = detSubscriptionResults.get(detID);