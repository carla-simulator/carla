function nextStops = readNextStops(result)
%readNextStops Internal function to read information about the next stops.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readNextStops.m 54 2019-01-03 15:41:54Z afacostag $

result.read(5);
numStops = result.readInt(); % Length
nextStops = cell(1,numStops);
for i=1:numStops
    result.read(1);
    lane = result.readString();
    result.read(1);
    endPos = result.readDouble();
    result.read(1);
    stoppingPlace = result.readString();
    result.read(1);
    stopFlags = result.readInt();
    result.read(1);
    duration = result.readDouble();
    result.read(1);
    until = result.readDouble();
    nextStops{i} = {lane, endPos, stoppingPlace, stopFlags, duration, until};
end
