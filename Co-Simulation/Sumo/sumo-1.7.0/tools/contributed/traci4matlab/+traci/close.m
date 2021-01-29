function close()
%CLOSE Close the connection with the SUMO server.
%   CLOSE() Close the default connection with the SUMO server. If several
%   connections have to be closed, they must be enabled through the
%   traci.switch() command, and closed with CLOSE().

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: close.m 48 2018-12-26 15:35:20Z afacostag $

global connections message
import traci.constants

% Close and clear the tcp object
if ~isempty(connections)
    if isKey(connections,'') && ~isempty(connections(''))
        % Build the close command
        command = uint8(sscanf(constants.CMD_CLOSE,'%x'));
        message.queue = [message.queue command];
        message.string = [message.string uint8(1+1) command];
        
        % Send the close command
        traci.sendExact();
        
        activeConnection = connections('');
        activeConnection.socket.close();
        connections('') = [];
    end
end
