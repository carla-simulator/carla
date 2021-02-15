%% TRACI TEST: LAS PALMAS MULTIMODAL SCENARIO
%   This m-file shows how to use the Traci4Matlab's pedestrian component. 
%   This example is reproduced from the TraCI-Python pedestrian tutorial, 
%   see http://sumo.dlr.de/wiki/Tutorials/TraCIPedCrossing. If you
%   want to test a TraCI command, just uncomment it.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: inter_palmas_actuated.m 53 2019-01-03 15:18:31Z afacostag $

clear
close all
clc

import traci.constants

% Get the filename of the example scenario
[scenarioPath,~,~] = fileparts(which(mfilename));
% scenarioPath = [scenarioPath '\inter_palmas'];
cd(scenarioPath);

traci.start('sumo-gui -c ./inter_palmas_actuated.sumocfg --start');

SIM_STEPS = [1 3600];
beginTime = SIM_STEPS(1);
duration =  SIM_STEPS(2);
endTime =  SIM_STEPS(1) +  SIM_STEPS(2) - 1;

% minimum green time for the vehicles
MIN_GREEN_TIME = 20;

% the first phase in tls plan. see 'pedcrossing.tll.xml'
VEHICLE_GREEN_PHASE = 0;

% the id of the traffic light (there is only one). This is identical to the
% id of the controlled intersection (by default)
TLSID = 'sanDiego';

% pedestrian edges at the controlled intersection
WALKINGAREAS = {':cluster_-427_339061698_w0', ':cluster_-427_339061698_w1'};
CROSSINGS = {':cluster_-427_339061698_c0', ':cluster_-427_339061698_c1'};

% track the duration for which the green phase of the vehicles has been active
greenTimeSoFar = 0;

% whether the pedestrian button has been pressed
activeRequest = 0;

% Initialize the vectors where the emissions will be recorded
greenPhase = ones(1,duration);
pedAccum = zeros(1,duration);
t = 50400:(54000 + duration -1);
tlsSwitch = 0;

% Variable for managing subscription to a person
subscribedTo10 = 0;
pedsInSim = {};

pedestrianCommandsTested = false;

% main loop.
for i = 1: duration
    traci.simulation.step();
    
    pedAccum(i) = length(traci.edge.getLastStepPersonIDs(':cluster_-427_339061698_w0'))...
        + length(traci.edge.getLastStepPersonIDs(':cluster_-427_339061698_w1'));
    if traci.trafficlights.getPhase(TLSID) ~= VEHICLE_GREEN_PHASE
        greenPhase(i) = 0;
    end
    
    % In this block we will test the remaining pedestrian commands
%     pedsInSim = traci.person.getIDList();
%     noPedsInSim = traci.person.getIDCount();
%     
%     if ismember('10', pedsInSim)
%         if ~pedestrianCommandsTested
%             if ~subscribedTo10
%                 traci.person.subscribe('10');
% %                 traci.person.subscribeContext('10',...
% %                     constants.CMD_GET_VEHICLE_VARIABLE,20);
%                 subscribedTo10 = 1;
%             end
%             position3D10 = traci.person.getPosition3D('6')
%             angle10 = traci.person.getAngle('6');
%             fprintf('Angle of person 6: %d\n', angle10);
%             color10 = traci.person.getColor('10');
%             fprintf('Color of person 10: [%d %d %d]\n', color10(1),...
%                 color10(2), color10(3));
%             lane10 = traci.person.getLanePosition('10');
%             fprintf('Person 10 is in lane position: %d\n', lane10);
%             length10 = traci.person.getLength('10');
%             fprintf('Length of person 10: %d\n', length10);
%             waitingTime10 = traci.person.getWaitingTime('10')
%             minGap10 = traci.person.getMinGap('10');
%             fprintf('Gap of person 10: %d\n', minGap10);
%             nextEdge10 = traci.person.getNextEdge('10')
%             edges10 = traci.person.getEdges('10')
%             stage10 = traci.person.getStage('10')
%             remainingStages10 = traci.person.getRemainingStages('10')
%             vehicle10 = traci.person.getVehicle('10')
%             position10 = traci.person.getPosition('6');
%             fprintf('Position of person 10: [%d %d]\n', position10(1),...
%                 position10(2));
%             roadID10 = traci.person.getRoadID('10')
%             fprintf('Person 10 is walking on road: %s\n', roadID10);
%             speed10 = traci.person.getSpeed('10');
%             fprintf('Speed of person 10: %d\n', speed10);
%             type10 = traci.person.getTypeID('10');
%             fprintf('Type ID of person 10: %s\n', type10);
%             width10 = traci.person.getWidth('10');
%             fprintf('Width of person 10: %d\n', width10);

%             traci.person.removeStages('10')
%             traci.person.add('myPerson', '33463249#0', 0);
%             traci.person.appendWaitingStage('10', 10);
%             traci.person.appendWalkingStage('10', {'-465','-436#0'}, 0);
    %         traci.person.appendDrivingStage('10', '-461#1', '');
%             traci.person.removeStage('10',0);
    %         traci.person.rerouteTraveltime('10');
%             traci.person.moveToXY('6', '-436#1', 55.66, 205.22);
%             traci.person.setSpeed('10', 5);
%             traci.person.setType('10','DEFAULT_PEDTYPE');
%             traci.person.setWidth('10',0.7);
%             traci.person.setHeight('10', 2);
%             traci.person.setLength('10', 0.2);
%             traci.person.setMinGap('10', 1);
%             traci.person.setColor('10', [255 0 0]);

            % Using subscriptions
%             subsResults10 = traci.person.getSubscriptionResults('10');
%             roadID10fromSubs = subsResults10(constants.VAR_ROAD_ID);
%             lanePosition10fromSubs = subsResults10(constants.VAR_LANEPOSITION);
%             fprintf('From subscription - Person 10 is walking on road: %s\n',...
%                 roadID10fromSubs);
%             fprintf('From subscription - Person 10 is in lane position: %d\n',...
%                 lanePosition10fromSubs);
% 
%             % Context subscriptions
% %             contextSubsResults10 = traci.person.getContextSubscriptionResults('10');
%     %         
%             pedestrianCommandsTested = true;
%         end
%     end
    
    % decide wether there is a waiting pedestrian and switch if the green
    % phase for the vehicles exceeds its minimum duration
    if ~activeRequest
        activeRequest = checkWaitingPersons(WALKINGAREAS, CROSSINGS);
    end
    if traci.trafficlights.getPhase(TLSID) == VEHICLE_GREEN_PHASE
        greenTimeSoFar = greenTimeSoFar + 1;
        if greenTimeSoFar > MIN_GREEN_TIME
            % check whether someone has pushed the button
            
            if activeRequest
                % switch to the next phase
                traci.trafficlights.setPhase(TLSID, VEHICLE_GREEN_PHASE + 1);
                % reset state
                activeRequest = 0;
                greenTimeSoFar = 0;
                
                tlsSwitch = tlsSwitch + 1;
                
            end
        end
    end
end

traci.close()

if duration > 330
    timePlot = 1:360;
else
    timePlot = 1:duration;
end

plot(greenPhase(timePlot))
hold on;
plot(pedAccum(timePlot),'r')
title('Green signal and pedestrian accumulation');
xlabel('t')
ylabel('Number of pedestrians in walking area')
legend('Green signal for vehicles', 'Number of pedestrians on sidewalks')
