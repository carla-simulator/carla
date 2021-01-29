function ContextSubscriptionResults = getContextSubscriptionResults(typeID)
%getContextSubscriptionResults Get the context subscription results for the
%   last time step.
%   contextSubscriptionResults = getContextSubscriptionResults(TYPEID) 
%   Returns the context subscription results for the last time step and the
%   given vehicle type. If no vehicle type id is given, all subscription 
%   results are returned in a containers.Map data struccure. If the vehicle
%   type id is unknown or the subscription did for any reason return no 
%   data, 'None' is returned.
%   It is not possible to retrieve older subscription results than the ones
%   from the last time step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getContextSubscriptionResults.m 48 2018-12-26 15:35:20Z afacostag $

global typeSubscriptionResults
if isempty(typeSubscriptionResults)
    throw(MException('traci:FatalTraCIError',...
        'You have to subscribe to the variable'));
end
if nargin < 1
    typeID=None;
end
ContextSubscriptionResults = typeSubscriptionResults.getContext(typeID);