function subscribe(junctionID, varargin) 
%subscribe Subscribe to junction variable.
%   subscribe(JUNCTIONID) Subscribe to the VAR_POSITION value for the 
%   maximum allowed interval.
%   subscribe(JUNCTIONID,VARIDS) Subscribe to the values given in the cell 
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

global junctionSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'junction.subscribe';
p.addRequired('junctionID',@ischar)
p.addOptional('varIDs', {constants.VAR_POSITION}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(junctionID, varargin{:})
junctionID = p.Results.junctionID;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

junctionSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.junction);

junctionSubscriptionResults.reset()
traci.subscribe(constants.CMD_SUBSCRIBE_JUNCTION_VARIABLE,...
    subscriptionBegin, subscriptionEnd, junctionID, varIDs)