function c = connect(varargin)
%CONNECT Initialize the connection to the SUMO server.
%   CONNECT() Establish a connection to a TraCI-Server using the default
%   port 8813 and return the connection object. The connection is not saved
%   in the pool and not accessible via traci.switch. The label of the
%   connection is 'default'. The connection is attempted for a number of
%   retries of ten.
%
%   CONNECT(PORT) Establishes the connection to the TraCI-Server server in the
%   localhost:PORT socket. 
%
%   CONNECT(PORT,NUMRETRIES) Try the connection for the given number of
%   retries.
%   
%   CONNECT(...,HOST) Specify the ip address of the TraCI-Server server as a
%   string e.g. '192.168.1.15'.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id$

import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'traci.connect';
p.addOptional('port', 8813, @(x)isnumeric(x) && length(x)==1)
p.addOptional('numRetries', 10, @(x)isnumeric(x) && length(x)==1)
p.addOptional('host', '127.0.0.1', @ischar)
p.parse(varargin{:})

port = p.Results.port;
numRetries = p.Results.numRetries;
host = p.Results.host;

% Create the tcp object
c = traci.Socket();

% Connect to the SUMO server within the given number of retries
err = [];
for i=1:numRetries
    try
        c.connect(host, port);
    break
    catch err        
        fprintf('Could not connect to TraCI server at %s:%s. %s\n', host, port, err);
        pause(i)
    end
end
if ~isempty(err)
    disp([err.identifier, err.message]);
    return
end
