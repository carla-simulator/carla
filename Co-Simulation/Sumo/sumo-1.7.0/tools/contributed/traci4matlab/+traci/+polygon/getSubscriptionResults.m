function subscriptionResults = getSubscriptionResults(polygonID)
%getSubscriptionResults Get the subscription results for the last time step.
%   subscriptionResults = getSubscriptionResults(POLYGONID) Returns the 
%   subscription results for the last time step and the given polygon. If no 
%   polygon id is given, all subscription results are returned in a 
%   containers.Map data structure.
%   If the polygon id is unknown or the subscription did for any reason return
%   no data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSubscriptionResults.m 48 2018-12-26 15:35:20Z afacostag $

global polygonSubscriptionResults
if isempty(polygonSubscriptionResults)
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    polygonID = 'None';
end

subscriptionResults = polygonSubscriptionResults.get(polygonID);