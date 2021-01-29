function subscribeContext(poiID, domain, dist, varargin) 
%subscribeContext Subscribe to a poi's context variable.
%   subscribeContext(POIID,DOMAIN,DIST) Subscribe to the 
%   VAR_POSITION value of the SUMO objects that surround the poi specified by 
%   POIID at a distance given by DIST, for the maximum allowed interval. 
%   The type of objets that surround the poi are defined in the DOMAIN 
%   parameter. Note that not all the SUMO object types support the variable
%   VAR_POSITION. 
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
%   $Id: subscribeContext.m 53 2019-01-03 15:18:31Z afacostag $

global poiSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'poi.subscribeContext';
p.addRequired('poiID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.VAR_POSITION}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(poiID, domain, dist, varargin{:})
poiID = p.Results.poiID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

poiSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.poi);

poiSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_POI_CONTEXT,...
    subscriptionBegin, subscriptionEnd, poiID, domain, dist, varIDs)