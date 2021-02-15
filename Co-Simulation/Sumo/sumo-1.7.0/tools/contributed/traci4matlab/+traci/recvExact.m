function result = recvExact()
%recvExact Internal function to receive the response from SUMO server.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: recvExact.m 48 2018-12-26 15:35:20Z afacostag $

global connections

% recvlength = [];
% connections('').BytesAvailable

% Receive the total length of the response
% while length(recvlength) < 1
	
% 	We tried to address the issue that arises when the user closes
%	the SUMO GUI without closing the connection from the client through
%	this workaround. Unfortunately, it leads to an incredibly high
%	slowdown.
% 	connClosed = strcmp(system('netstat | findstr 8813'),'');
% 	if connClosed
% 		result = [];
% 		break
% 	end
% 	t = connections('').readInt();
% 	if isempty(t)
% 		result = [];
% 		return
% 	end
% 	recvlength = [recvlength t];
% end

activeConnection = connections('');
recvLength = activeConnection.dis.readInt() - 4;

% Receive the response

result = typecast(activeConnection.dataReader.readBuffer(recvLength),'uint8');

% while length(result) < recvlength
% 	t = fread(connections(''),recvlength-length(result),'uint8');
% % 	flushinput(connections(''));
% 	if isempty(t)
% 		result = [];
% 		return
% 	end
% 	result = [result t];
% end

% Construct the traci.Storage object containing the result
result = traci.Storage(uint8(result'));