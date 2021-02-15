function signals = readLinks(result)
%readLinks Internal function to read the links controlled by the traffic lights.   

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readLinks.m 49 2018-12-27 14:08:44Z afacostag $

result.readLength();
numSignals = result.readInt(); % Length
signals = cell(1,numSignals);
for i=1:numSignals
    result.read(1);                           % Type of Number of Controlled Links
    nbControlledLinks = result.readInt();       % Number of Controlled Links
    controlledLinks = cell(1,nbControlledLinks);
    for j=1:nbControlledLinks
        result.read(1);                       % Type of Link j
        link = result.readStringList();       % Link j
        controlledLinks{j} = link;
    end
    signals(i) = controlledLinks;
end