function subscribe(varargin) 
%subscribe Subscribe to simulation variable.
%   subscribe() Subscribe to the VAR_DEPARTED_VEHICLES_IDS value for
%   the maximum allowed interval.
%   subscribe(VARIDS) Subscribe to the values given in the cell 
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

global simSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'simulation.subscribe';
p.addOptional('varIDs', {constants.VAR_DEPARTED_VEHICLES_IDS}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(varargin{:})
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

simSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.simulation);

simSubscriptionResults.reset()
traci.subscribe(constants.CMD_SUBSCRIBE_SIM_VARIABLE,...
    subscriptionBegin, subscriptionEnd, 'x', varIDs)