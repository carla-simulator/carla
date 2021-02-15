function result = sendExact()
%sendExact An internal function to send a message to the SUMO server and
%parse the result.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: sendExact.m 48 2018-12-26 15:35:20Z afacostag $

global message connections
import traci.constants
% warning('off','instrument:fread:unsuccessfulRead');

% Length of the command
len = 4 + length(message.string);
activeConnection = connections('');

% Write the message to the tcp socket
activeConnection.dos.writeInt(len);
activeConnection.dos.write(message.string);

% Read the result from the socket
result = traci.recvExact();

if isempty(result)
    fclose(connections(''));
    clear connections('')
    throw(MException('traci:FatalTraciError','Connection closed by SUMO\n'))
end

% Parse the result
for i= 1:length(message.queue)
	prefix = result.read(3);
	if prefix(3)==0
		strresult = 'OK';
	elseif prefix(3)==1
		strresult = 'Not Implemented';
	else
		strresult = 'Error';
	end
	err = result.readString();
	if prefix(3) || ~isempty(err)
        message.string = [];
        message.queue = [];
		traci.close();
		throw(MException('traci:FatalTraciError','%s %s %s\n', num2str(prefix), strresult, err));
	elseif prefix(2) ~= message.queue(i)
		traci.close();
		throw(MException('traci:FatalTraciError','Received answer 0x%.2X for command 0x%.2x.\n',...
            prefix(2), message.queue(i)));
	elseif strcmp(prefix(2),constants.CMD_STOP)
		len = result.read(1) - 1;
		result.read(len + 1);
	end
end

% Clear the message contents
message.string = [];
message.queue = [];



