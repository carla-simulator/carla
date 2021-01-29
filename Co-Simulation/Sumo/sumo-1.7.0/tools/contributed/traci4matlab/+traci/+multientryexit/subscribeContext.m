function subscribeContext(detID, domain, dist, varargin) 
%subscribeContext Subscribe to a multi-entry/multi-exit detector's context 
%   variable.
%   subscribeContext(EDGEID,DOMAIN,DIST) Subscribe to the 
%   LAST_STEP_VEHICLE_NUMBER value of the SUMO objects that surround the 
%   multi-entry/multi-exit detector specified by EDGEID at a distance given
%   by DIST, for the maximum allowed interval. The type of objets that 
%   surround the multi-entry/multi-exit detector are defined in the DOMAIN 
%   parameter. Note that not all the SUMO object types support the variable
%   LAST_STEP_VEHICLE_NUMBER. 
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

global detSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'multientryexit.subscribeContext';
p.addRequired('detID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.LAST_STEP_VEHICLE_NUMBER}, @iscell)
p.addOptional('subscriptionBegin', 0, @(x)isnumeric(x) && length(x)==1)
p.addOptional('subscriptionEnd', 2^31-1, @(x)isnumeric(x) && length(x)==1)
p.parse(detID, domain, dist, varargin{:})
detID = p.Results.detID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

detSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.multientryexit);

detSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_CONTEXT,...
    subscriptionBegin, subscriptionEnd, detID, domain, dist, varIDs)