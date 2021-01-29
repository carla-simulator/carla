function subscribeContext(edgeID, domain, dist, varargin) 
%subscribeContext Subscribe to an edge's context variable.
%   subscribeContext(EDGEID,DOMAIN,DIST) Subscribe to the 
%   LAST_STEP_VEHICLE_NUMBER value of the SUMO objects that surround the 
%   edge specified by EDGEID at a distance given by DIST, for the maximum 
%   allowed interval. The type of objets that surround the edge are defined
%   in the DOMAIN parameter. Note that not all the SUMO object types 
%   support the variable LAST_STEP_VEHICLE_NUMBER. 
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

global edgeSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'edge.subscribeContext';
p.addRequired('edgeID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.LAST_STEP_VEHICLE_NUMBER}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(edgeID, domain, dist, varargin{:})
edgeID = p.Results.edgeID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

% Construct the subscription results object
if isempty(edgeSubscriptionResults)
    edgeSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.edge);
else
    edgeSubscriptionResults.reset();
end

% Call the traci subscribeContext function
traci.subscribeContext(constants.CMD_SUBSCRIBE_EDGE_CONTEXT,...
    subscriptionBegin, subscriptionEnd, edgeID, domain, dist, varIDs);
