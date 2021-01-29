function links = readLinks(result)
%readLinks Internal function to read information about the links.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readLinks.m 48 2018-12-26 15:35:20Z afacostag $

result.read(5) % Type Compound, Length
nbLinks = result.readInt();
links = {};
for i=1:nbLinks
    result.read(1);                           % Type String
    approachedLane = result.readString();
    result.read(1);                           % Type String
    approachedInternal = result.readString();
    result.read(1);                           % Type Byte
    hasPrio = result.read(1);
    result.read(1);                           % Type Byte
    isOpen = result.read(1);
    result.read(1);                           % Type Byte
    hasFoe = result.read(1);
    result.read(1);                           % Type String
    state = result.readString(); 
    result.read(1);                           % Type String
    direction = result.readString(); 
    result.read(1);                           % Type Float
    len = result.readDouble();
    links = [links approachedLane, hasPrio, isOpen, hasFoe];
end