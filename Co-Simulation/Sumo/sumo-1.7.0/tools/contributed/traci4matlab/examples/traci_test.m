%% SUMO TRACI TEST
% THIS FILE AIMS TO TEST THE TRACI PROTOCOL FOR SUMO.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: traci_test.m 55 2019-01-03 21:02:25Z afacostag $

clear all
close all
clc

%% MAIN
% HERE, WE START THE SUMO SIMULATOR AND INITIALIZE IT, ACCORDING TO
% THE EXAMPLE FOUND IN http://sumo-sim.org/userdoc/Tutorials/TraCI4Traffic_Lights.html

% This line is required for Matlab on Linux
% setenv('LD_LIBRARY_PATH','/usr/lib/x86_64-linux-gnu/libxerces-c-3.1.so');

scenarioPath = './inter_palmas/tls_actuated/inter_palmas_actuated.sumocfg ';

[traciVersion,sumoVersion] = traci.start(['sumo -c ' '"' scenarioPath '"']);

traci.close();
fprintf('SUMO version: %s\nTraCI version: %d\n',sumoVersion,traciVersion);
