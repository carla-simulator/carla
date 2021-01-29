function [activeRequest, numPeds] = checkWaitingPersons(WALKINGAREAS, CROSSINGS)
% Function used by the TraCI4Matlab pedestrian test to check whether a
%   person has requested to cross the street.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: checkWaitingPersons.m 52 2019-01-02 15:20:39Z afacostag $

% check both sides of the crossing
for i = 1:length(WALKINGAREAS)
    numPeds = traci.edge.getLastStepPersonIDs(WALKINGAREAS{i});
    % check who is waiting at the crossing
    % we assume that pedestrians push the button upon
    % standing still for 1s
    for j = 1:length(numPeds)
        if traci.person.getWaitingTime(numPeds{j}) == 1 &&...
                ismember(traci.person.getNextEdge(numPeds{j}),CROSSINGS)
            fprintf('%s pushes the button\n',  numPeds{j});
            activeRequest = 1;
            return
        end
    end
    activeRequest = 0;
end