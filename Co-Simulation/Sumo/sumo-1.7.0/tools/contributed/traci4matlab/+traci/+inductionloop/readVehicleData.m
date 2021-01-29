function data = readVehicleData(result)
%readVehicleData Internal function to read several vehicle information.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readVehicleData.m 48 2018-12-26 15:35:20Z afacostag $

result.readLength();
nbData = result.readInt();
data = {};
for i=1:nbData
    result.read(1);
    vehID = result.readString();
    result.read(1);
    len = result.readDouble();
    result.read(1);
    entryTime = result.readDouble();
    result.read(1);
    leaveTime = result.readDouble();
    result.read(1);
    typeID = result.readString();
    data = [data, vehID, len, entryTime, leaveTime, typeID];
end