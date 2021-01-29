function switchConnection(label)
%switchConnection Switch to another connection with SUMO.
%
%   switchConnection(label) Switch to the connection specified in LABEL 
%   with the SUMO server.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: switchConnection.m 48 2018-12-26 15:35:20Z afacostag $

global connections
connections('') = connections(label);