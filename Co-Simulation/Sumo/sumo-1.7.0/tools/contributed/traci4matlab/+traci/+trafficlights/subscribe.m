function subscribe(tlsID, varargin) 
%subscribe Subscribe to traffic lights variable.
%   subscribe(TLSID) Subscribe to the TL_CURRENT_PHASE value for the 
%   maximum allowed interval.
%   subscribe(TLSID,VARIDS) Subscribe to the values given in the cell 
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

global tlSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'trafficlights.subscribe';
p.addRequired('tlsID',@ischar)
p.addOptional('varIDs', {constants.TL_CURRENT_PHASE}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(tlsID, varargin{:})
tlsID = p.Results.tlsID;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

tlSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.trafficlights);

tlSubscriptionResults.reset()
traci.subscribe(constants.CMD_SUBSCRIBE_TL_VARIABLE,...
    subscriptionBegin, subscriptionEnd, tlsID, varIDs)