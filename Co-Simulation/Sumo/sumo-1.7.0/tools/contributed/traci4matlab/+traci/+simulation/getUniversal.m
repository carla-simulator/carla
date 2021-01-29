function returnedValue = getUniversal(varID, varargin)
%getUniversal An internal function to send the get command and read the 
%variable value.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getUniversal.m 50 2018-12-28 16:25:47Z afacostag $

import traci.constants
global simSubscriptionResults

objID = '';
if nargin > 1
    objID = varargin{1};
end

if isempty(simSubscriptionResults)
    returnValueFunc = traci.RETURN_VALUE_FUNC.simulation;
else
    returnValueFunc = simSubscriptionResults.valueFunc;
end

% Prepare the outgoing message and read the response. The result variable
% is a traci.Storage object
result = traci.sendReadOneStringCmd(constants.CMD_GET_SIM_VARIABLE,varID,objID);
handleReturValueFunc = str2func(returnValueFunc(varID));

% Use the proper method to read the variable of interest from the result
returnedValue = handleReturValueFunc(result);