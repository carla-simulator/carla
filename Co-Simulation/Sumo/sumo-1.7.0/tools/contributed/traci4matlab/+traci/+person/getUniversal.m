function returnedValue = getUniversal(varID, personID)
%getUniversal An internal function to send the get command and read the 
%variable value.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getUniversal.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
global personSubscriptionResults

if isempty(personSubscriptionResults)
    returnValueFunc = traci.RETURN_VALUE_FUNC.person;
else
    returnValueFunc = personSubscriptionResults.valueFunc;
end

% Prepare the outgoing message and read the response. The result variable
% is a traci.Storage object
result = traci.sendReadOneStringCmd(constants.CMD_GET_PERSON_VARIABLE,varID,personID);
handleReturValueFunc = str2func(returnValueFunc(varID));

% Use the proper method to read the variable of interest from the result
returnedValue = handleReturValueFunc(result);