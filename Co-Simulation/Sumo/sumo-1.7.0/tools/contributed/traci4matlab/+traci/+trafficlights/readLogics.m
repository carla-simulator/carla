function logics = readLogics(result)
%readLogics Internal function to read the complete program of the traffic lights.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: readLogics.m 49 2018-12-27 14:08:44Z afacostag $

numLogics = result.readInt();
logics = cell(1,numLogics);
for i=1:numLogics
    result.readCompound(5);
    programID = result.readTypedString();
    type = result.readTypedInt();
    currentPhaseIndex = result.readTypedInt();
    logic = traci.trafficlights.Logic(programID, type, currentPhaseIndex);
    numPhases = result.readCompound();
    phases = cell(1, numPhases);
    for j=1:numPhases
        result.readCompound(5);
        duration = result.readTypedDouble();
        state = result.readTypedString();
        minDur = result.readTypedDouble();
        maxDur = result.readTypedDouble();
        next = result.readTypedInt();
        phases{j} = traci.trafficlights.Phase(duration, state, minDur, maxDur, next);
    end
    logic.phases = phases;
    numParams = result.readCompound();
    for j = 1:numParams
        kv = result.readTypedStringList();
        logic.subParameter(kv{1}) = kv{2};
    end
    logics{i} = logic;
end