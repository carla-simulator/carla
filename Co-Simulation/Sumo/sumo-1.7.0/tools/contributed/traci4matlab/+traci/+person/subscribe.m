function subscribe(personID, varargin) 
%subscribe Subscribe to person variable.
%   subscribe(PERSONID) Subscribe to the VAR_ROAD_ID and VAR_LANEPOSITION 
%   values for the maximum allowed interval.
%   subscribe(PERSONID,VARIDS) Subscribe to the values given in the cell 
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

global personSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'person.subscribe';
p.addRequired('personID',@ischar)
p.addOptional('varIDs', {constants.VAR_ROAD_ID, constants.VAR_LANEPOSITION}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(personID, varargin{:})
personID = p.Results.personID;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

personSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.person);

personSubscriptionResults.reset()
traci.subscribe(constants.CMD_SUBSCRIBE_PERSON_VARIABLE,...
    subscriptionBegin, subscriptionEnd, personID, varIDs)