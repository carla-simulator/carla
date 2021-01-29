function subscribeContext(loopID, domain, dist, varargin) 
%subscribeContext Subscribe to an induction loop's context variable.
%   subscribeContext(LOOPID,DOMAIN,DIST) Subscribe to the 
%   LAST_STEP_VEHICLE_NUMBER value of the SUMO objects that surround the 
%   induction loop specified by LOOPID at a distance given by DIST, for the
%   maximum allowed interval. The type of objets that surround the 
%   induction loop are defined in the DOMAIN parameter. Note that not all 
%   the SUMO object types support the variable LAST_STEP_VEHICLE_NUMBER. 
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

global loopSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'inductionloop.subscribeContext';
p.addRequired('loopID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.LAST_STEP_VEHICLE_NUMBER}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(loopID, domain, dist, varargin{:})
loopID = p.Results.loopID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

loopSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.inductionloop);

loopSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT,...
    subscriptionBegin, subscriptionEnd, loopID, domain, dist, varIDs)