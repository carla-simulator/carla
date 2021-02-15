function subscriptionResults = getSubscriptionResults(edgeID)
%getSubscriptionResults Get the subscription results for the last time step.
%   subscriptionResults = getSubscriptionResults(EDGEID) Returns the 
%   subscription results for the last time step and the given edge. If no 
%   edge id is given, all subscription results are returned in a 
%   containers.Map data structure.
%   If the edge id is unknown or the subscription did for any reason return
%   no data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSubscriptionResults.m 53 2019-01-03 15:18:31Z afacostag $

global edgeSubscriptionResults
if isempty(edgeSubscriptionResults)
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    edgeID = 'None';
end

subscriptionResults = edgeSubscriptionResults.get(edgeID);
