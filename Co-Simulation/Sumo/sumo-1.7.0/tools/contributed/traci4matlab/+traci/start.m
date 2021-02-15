function [traciVersion, sumoVersion] = start(varargin)
%START Initialize the connection to the SUMO server.
%   START(CMD) Establish a connection to a TraCI-Server and store it under
%   the label 'default'. The connection is attempted 
%   for a number of retries of ten.
%
%   START(CMD, PORT) Initialize the connection to the SUMO server in the
%   localhost:PORT socket. 
%
%   START(...,NUMRETRIES) Try the connection for the given number of
%   retries.
%   
%   START(...,HOST) Specify the ip address of the SUMO server as a string
%   e.g. '192.168.1.15'.
%
%   START(...,LABEL) Specify a label for the connection.
%
%   [TRACIVERSION,SUMOVERSION] = START(...) Returns the TRACIVERSION and the
%   SUMOVERSION.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id$

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
p.FunctionName = 'traci.start';
p.addRequired('cmd', @ischar)
p.addOptional('port', 8813, @(x)isnumeric(x) && length(x)==1)
p.addOptional('numRetries', 10, @(x)isnumeric(x) && length(x)==1)
p.addOptional('host', '127.0.0.1', @ischar)
p.addOptional('label', 'default', @ischar)
p.parse(varargin{:})

cmd = p.Results.cmd;
port = p.Results.port;
numRetries = p.Results.numRetries;
host = p.Results.host;
label = p.Results.label;

system([cmd ' --remote-port ' num2str(port) '&']);

% Create the tcp object
if isempty(connections)
    connections = containers.Map();
end

connections(label) = traci.connect(port, numRetries, host);
traci.switchConnection(label)

[traciVersion,sumoVersion] = traci.getVersion();