function subscribeContext(tlsID, domain, dist, varargin) 
%subscribeContext Subscribe to a traffic lights's context variable.
%   subscribeContext(TLSID,DOMAIN,DIST) Subscribe to the TL_CURRENT_PHASE 
%   value of the SUMO objects that surround the traffic lights specified by
%   TLSID at a distance given by DIST, for the maximum allowed interval. 
%   The type of objets that surround the traffic lights are defined in the 
%   DOMAIN parameter. Note that not all the SUMO object types support the 
%   variable TL_CURRENT_PHASE. 
%   subscribeContext(...,VARIDS) Subscribe to the values given in the cell 
%   array of strings VARIDS.
%   subscribeContext(...,BEGIN) Subscribe from the time BEGIN to the
%   maximum allowed end time.
%   subscribeContext(...,END) Subscribe for the time interval defined by 
%   BEGIN and END. 
%   A call to this method clears all previous subscription results.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: subscribeContext.m 48 2018-12-26 15:35:20Z afacostag $

global tlsSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'trafficlights.subscribeContext';
p.addRequired('tlsID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.TL_CURRENT_PHASE}, @iscell)
p.addOptional('subscriptionBegin', 0, @(x)isnumeric(x) && length(x)==1)
p.addOptional('subscriptionEnd', 2^31-1, @(x)isnumeric(x) && length(x)==1)
p.parse(tlsID, domain, dist, varargin{:})
tlsID = p.Results.tlsID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

tlsSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.trafficlights);

tlsSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_TL_CONTEXT,...
    subscriptionBegin, subscriptionEnd, tlsID, domain, dist, varIDs)