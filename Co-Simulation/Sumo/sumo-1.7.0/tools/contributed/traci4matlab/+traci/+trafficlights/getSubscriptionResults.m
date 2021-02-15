function subscriptionResults = getSubscriptionResults(tlsID)
%getSubscriptionResults Get the subscription results for the last time step.
%   subscriptionResults = getSubscriptionResults(TLSID) Returns the 
%   subscription results for the last time step and the given traffic 
%   lights. If no traffic lights id is given, all subscription results are 
%   returned in a containers.Map data structure.
%   If the traffic lights id is unknown or the subscription did for any reason return
%   no data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getSubscriptionResults.m 48 2018-12-26 15:35:20Z afacostag $

global tlSubscriptionResults
if isempty(tlSubscriptionResults)
	traci.close();
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    tlsID = 'None';
end

subscriptionResults = tlSubscriptionResults.get(tlsID);