function subscribe(routeID, varargin) 
%subscribe Subscribe to route variable.
%   subscribe(ROUTEID) Subscribe to the ID_LIST value for the maximum 
%   allowed interval.
%   subscribe(ROUTEID,VARIDS) Subscribe to the values given in the cell 
%   array of strings VARIDS for the maximum allowed interval.
%   subscribe(...,BEGIN) Subscribe from the time BEGIN to the maximum 
%   allowed end time.
%   subscribe(...,END) Subscribe for the time interval defined by BEGIN and 
%   END.
%   A call to this function clears all previous subscription results.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: subscribe.m 53 2019-01-03 15:18:31Z afacostag $

global routeSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'route.subscribe';
p.addRequired('routeID',@ischar)
p.addOptional('varIDs', {constants.ID_LIST}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(routeID, varargin{:})
routeID = p.Results.routeID;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

routeSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.route);

routeSubscriptionResults.reset()
traci.subscribe(constants.CMD_SUBSCRIBE_ROUTE_VARIABLE,...
    subscriptionBegin, subscriptionEnd, routeID, varIDs)