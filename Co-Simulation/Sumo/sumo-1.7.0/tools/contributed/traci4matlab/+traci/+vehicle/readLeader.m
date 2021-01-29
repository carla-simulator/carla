function [vehID, dist] = readLeader(result)

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readLeader.m 48 2018-12-26 15:35:20Z afacostag $

result.readInt();
result.read(1);
vehID = result.readString();
result.read(1);
dist = result.readDouble();




