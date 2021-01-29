function subscribeContext(vehID, domain, dist, varargin) 
%subscribeContext Subscribe to a vehicle's context variable.
%   subscribeContext(VEHID,DOMAIN,DIST) Subscribe to the VAR_ROAD_ID and 
%   VAR_LANEPOSITION value of the SUMO objects that surround the vehicle 
%   specified by VEHID at a distance given by DIST, for the maximum allowed
%   interval. The type of objets that surround the vehicle are defined in 
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
%   $Id: subscribeContext.m 53 2019-01-03 15:18:31Z afacostag $

global vehSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'vehicle.subscribeContext';
p.addRequired('vehID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.VAR_ROAD_ID, constants.VAR_LANEPOSITION}, @iscell)
p.addOptional('subscriptionBegin', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.addOptional('subscriptionEnd', constants.INVALID_DOUBLE_VALUE,...
    @(x)(isnumeric(x) && length(x)==1))
p.parse(vehID, domain, dist, varargin{:})
vehID = p.Results.vehID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

vehSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.vehicle);

vehSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_VEHICLE_CONTEXT,...
    subscriptionBegin, subscriptionEnd, vehID, domain, dist, varIDs)