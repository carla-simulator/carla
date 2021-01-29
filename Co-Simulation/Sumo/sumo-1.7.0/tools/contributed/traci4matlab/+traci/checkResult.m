function result = checkResult(cmdID, varID, objID)
%checkResult Internal function to receive and parse the response from SUMO
%server.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: checkResult.m 48 2018-12-26 15:35:20Z afacostag $

% Send the message and read the result
result = traci.sendExact();

% Read the response length
result.readLength();

% Read the response and the id of the returned variable
response = result.read(1);
retVarID = result.read(1);

% Read the acknowledged object ID
objectID = result.readString();

% Check whether the response corresponds to the requested command. The
% TraCI protocol is designed so that the response and the command ID
% difference is 16
if response - uint8(sscanf(cmdID,'%x')) ~= 16 || retVarID ~= uint8(sscanf(varID,'%x'))...
		|| ~strcmp(objectID,objID)
	fprintf('Received answer %s,%s,%s for command %s,%s,%s.'...
		,response, retVarID, objectID, cmdID, varID, objID);
end

% Read the type of the variable
result.read(1);