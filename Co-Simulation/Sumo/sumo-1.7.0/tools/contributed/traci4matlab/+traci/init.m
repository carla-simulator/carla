function [traciVersion, sumoVersion] = init(varargin)
%INIT Initialize the connection to the SUMO server.
%   INIT() Initialize the connection to the SUMO server using the socket
%   localhost:8813. The SUMO server must be listening on the same socket.
%   The label of the connection is 'default'. The connection is attempted 
%   for a number of retries of ten.
%
%   INIT(PORT) Initialize the connection to the SUMO server in the
%   localhost:PORT socket. 
%
%   INIT(PORT,NUMRETRIES) Try the connection for the given number of
%   retries.
%   
%   INIT(...,HOST) Specify the ip address of the SUMO server as a string
%   e.g. '192.168.1.15'.
%
%   INIT(...,LABEL) Specify a label for the connection.
%
%   [TRACIVERSION,SUMOVERSION] = INIT(...) Returns the TRACIVERSION and the
%   SUMOVERSION.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: init.m 48 2018-12-26 15:35:20Z afacostag $

% Add the DataReader class
% [pathstr,~,~] = fileparts(which('traci.init'));

% dynJavaPath =  javaclasspath('-dynamic');
% isInPath = ismember(dynJavaPath,[pathstr '\..\traci4matlab.jar']);
% if(~any(isInPath) || isempty(isInPath))
%     javaaddpath([pathstr '\..\traci4matlab.jar']);
% end

global connections

import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'traci.init';
p.addOptional('port', 8813, @(x)isnumeric(x) && length(x)==1)
p.addOptional('numRetries', 10, @(x)isnumeric(x) && length(x)==1)
p.addOptional('host', '127.0.0.1', @ischar)
p.addOptional('label', 'default', @ischar)
p.parse(varargin{:})

port = p.Results.port;
numRetries = p.Results.numRetries;
host = p.Results.host;
label = p.Results.label;

% Create the tcp object
if isempty(connections)
    connections = containers.Map();
end
connections('') = traci.Socket();
connections(label) = connections('');

% Connect to the SUMO server within the given number of retries
err = [];
for i=1:numRetries
    try
        connections(label).connect(host, port);
    break
    catch err        
        pause(i)
    end
end
if ~isempty(err)
    disp([err.identifier, err.message]);
end

[traciVersion,sumoVersion] = traci.getVersion();