function subscribeContext(typeID, domain, dist, varargin) 
%subscribeContext Subscribe to a vehicle type's context variable.
%   subscribeContext(TYPEID,DOMAIN,DIST) Subscribe to the VAR_MAXSPEED 
%   value of the SUMO objects that surround the vehicle type specified by 
%   TYPEID at a distance given by DIST, for the maximum allowed interval. 
%   The type of objets that surround the vehicle type are defined in the 
%   DOMAIN parameter. Note that not all the SUMO object types support the 
%   variable VAR_MAXSPEED. 
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

global typeSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'type.subscribeContext';
p.addRequired('typeID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.VAR_MAXSPEED}, @iscell)
p.addOptional('subscriptionBegin', 0, @(x)isnumeric(x) && length(x)==1)
p.addOptional('subscriptionEnd', 2^31-1, @(x)isnumeric(x) && length(x)==1)
p.parse(typeID, domain, dist, varargin{:})
typeID = p.Results.typeID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

typeSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.vehicletype);

typeSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_VEHICLETYPE_CONTEXT,...
    subscriptionBegin, subscriptionEnd, typeID, domain, dist, varIDs)