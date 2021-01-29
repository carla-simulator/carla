function subscribeContext(routeID, domain, dist, varargin) 
%subscribeContext Subscribe to a route's context variable.
%   subscribeContext(ROUTEID,DOMAIN,DIST) Subscribe to the ID_LIST value of
%   the SUMO objects that surround the route specified by ROUTEID at a 
%   distance given by DIST, for the maximum allowed interval. The type of 
%   objets that surround the route are defined in the DOMAIN parameter. 
%   Note that not all the SUMO object types support the variable ID_LIST. 
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

global routeSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'route.subscribeContext';
p.addRequired('routeID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.ID_LIST}, @iscell)
p.addOptional('subscriptionBegin', 0, @(x)isnumeric(x) && length(x)==1)
p.addOptional('subscriptionEnd', 2^31-1, @(x)isnumeric(x) && length(x)==1)
p.parse(routeID, domain, dist, varargin{:})
routeID = p.Results.routeID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

routeSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.route);

routeSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_ROUTE_CONTEXT,...
    subscriptionBegin, subscriptionEnd, routeID, domain, dist, varIDs)