function returnedValue = getUniversal(varID, detID)
%getUniversal An internal function to send the get command and read the 
%variable value.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getUniversal.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global detSubscriptionResults

if isempty(detSubscriptionResults)
    returnValueFunc = traci.RETURN_VALUE_FUNC.multientryexit;
else
    returnValueFunc = detSubscriptionResults.valueFunc;
end

% Prepare the outgoing message and read the response. The result variable
% is a traci.Storage object
result = traci.sendReadOneStringCmd(constants.CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE,varID,detID);
handleReturValueFunc = str2func(returnValueFunc(varID));

% Use the proper method to read the variable of interest from the result
returnedValue = handleReturValueFunc(result);