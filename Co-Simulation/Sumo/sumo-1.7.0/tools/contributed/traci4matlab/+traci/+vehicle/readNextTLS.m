function nextTLSs = readNextTLS(result)
%readNextTLS Internal function to read information about the upcoming
%traffic lights.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readNextTLS.m 48 2018-12-26 15:35:20Z afacostag $

result.read(5);
numTLS = result.readInt(); % Length
nextTLSs = cell(1,numTLS);
for i=1:numTLS
    result.read(1);
    tlsID = result.readString();
    result.read(1);
    tlsIndex = result.readInt();
    result.read(1);
    dist = result.readDouble();
    result.read(1);
    state = result.read(1);
    nextTLSs{i} = {tlsID, tlsIndex, dist, char(state)};
end
