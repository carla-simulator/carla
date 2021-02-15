function subscribe(polygonID, varargin) 
%subscribe Subscribe to polygon variable.
%   subscribe(POLYGONID) Subscribe to the VAR_SHAPE value for the maximum 
%   allowed interval.
%   subscribe(POLYGONID,VARIDS) Subscribe to the values given in the cell 
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

global polygonSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'polygon.subscribe';
p.addRequired('polygonID',@ischar)
p.addOptional('varIDs', {constants.VAR_SHAPE}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(polygonID, varargin{:})
polygonID = p.Results.polygonID;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

polygonSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.polygon);

polygonSubscriptionResults.reset()
traci.subscribe(constants.CMD_SUBSCRIBE_POLYGON_VARIABLE,...
    subscriptionBegin, subscriptionEnd, polygonID, varIDs)