function subscribeContext(junctionID, domain, dist, varargin) 
%subscribeContext Subscribe to a junction's context variable.
%   subscribeContext(JUNCTIONID,DOMAIN,DIST) Subscribe to the 
%   VAR_POSITION value of the SUMO objects that surround the junction 
%   specified by JUNCTIONID at a distance given by DIST, for the maximum 
%   allowed interval. The type of objets that surround the junction are 
%   defined in the DOMAIN parameter. Note that not all the SUMO object 
%   types support the variable VAR_POSITION. 
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

global junctionSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'junction.subscribeContext';
p.addRequired('junctionID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.VAR_POSITION}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(junctionID, domain, dist, varargin{:})
junctionID = p.Results.junctionID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

junctionSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.junction);

junctionSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_JUNCTION_CONTEXT,...
    subscriptionBegin, subscriptionEnd, junctionID, domain, dist, varIDs)