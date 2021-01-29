function subscribeContext(personID, domain, dist, varargin) 
%subscribeContext Subscribe to a person's context variable.
%   subscribeContext(PERSONID,DOMAIN,DIST) Subscribe to the VAR_ROAD_ID and 
%   VAR_LANEPOSITION value of the SUMO objects that surround the person 
%   specified by PERSONID at a distance given by DIST, for the maximum allowed
%   interval. The type of objets that surround the person are defined in 
%   the DOMAIN parameter. Note that not all the SUMO object types support 
%   the variables VAR_ROAD_ID and VAR_LANEPOSITION. 
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

global personSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'person.subscribeContext';
p.addRequired('personID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.VAR_ROAD_ID, constants.VAR_LANEPOSITION}, @iscell)
p.addOptional('subscriptionBegin', 0, @(x)isnumeric(x) && length(x)==1)
p.addOptional('subscriptionEnd', 2^31-1, @(x)isnumeric(x) && length(x)==1)
p.parse(personID, domain, dist, varargin{:})
personID = p.Results.personID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

personSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.person);

personSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_PERSON_CONTEXT,...
    subscriptionBegin, subscriptionEnd, personID, domain, dist, varIDs)