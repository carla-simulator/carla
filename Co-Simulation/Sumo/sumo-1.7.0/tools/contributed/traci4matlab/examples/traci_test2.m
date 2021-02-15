%% TRACI TEST 2
%   This m-file shows how to use Traci4Matlab.
%   This example uses the files of the SUMO traci tutorial, see
%   http://sumo-sim.org/userdoc/Tutorials/TraCI4Traffic_Lights.html. If you
%   want to test a TraCI command, just uncomment it. The commands are 
%   organized by SUMO object type, some of them needed to be included in 
%   the main loop of the script. Note that you have to generate your own
%   traffic demand, we recommend to obtain it by running tha SUMO TraCI
%   tutorial using Python 

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: traci_test2.m 53 2019-01-03 15:18:31Z afacostag $

clear all
close all
clc

%% MAIN
% The scenario consists of a single intersection controlled by a traffic light
% whose phases are changed if vehicles pass through an induction loop.

% Initialize SUMO from the system. note that it is initialized in graphical
% interface mode. You have to set the SUMO_HOME environment variable
% pointing to your SUMO root directory and modify the windows path to 
% include the %SUMO_HOME%/bin directory.

% For linux users
% setenv('LD_LIBRARY_PATH','/usr/lib/x86_64-linux-gnu/libxerces-c-3.1.so');
% setenv('SUMO_HOME', '/home/afacostag/sumo');

docsRelPath1 = '/docs/tutorial/traci_tls/data';
docsRelPath2 = '/doc/tutorial/traci_tls/data';
testsRelPath = '/tests/complex/tutorial/traci_tls/data/cross.sumocfg';

% Tutorial in docs
scenarioPath1 = fullfile([getenv('SUMO_HOME') docsRelPath1],'cross.sumocfg');
scenarioPath2 = fullfile([getenv('SUMO_HOME') docsRelPath2],'cross.sumocfg');

if ~isempty(dir(scenarioPath1))
  scenarioPath = scenarioPath1;
elseif ~isempty(dir(scenarioPath2))
  scenarioPath = scenarioPath2;
else
  % Tutorial in tests
  scenarioPath = fullfile([getenv('SUMO_HOME') testsRelPath]);
end

simRuns = 10;
simDurations = zeros(simRuns, 1);

%for simRun = 1:simRuns

%tic;

traci.start(['sumo-gui -c ' '"' scenarioPath '"' ' --start']);

% To test vehicle commands, we have to check wether the sumo 0.20.0
% version is installed, because in that version the prefix of the vehicle
% names has changed.
sumoHome = getenv('SUMO_HOME');
if isempty(strfind(sumoHome,'-'))
  sumoVersion = 'unknown';
  %     testVehicle = '10';
  testVehicle = 'right_25';
else
  sumoVersion = str2double(sumoHome(9:12));
  if sumoVersion < 0.20
%     testVehicle = '10';
    testVehicle = '25';
  else
%     testVehicle = 'right_10';
    testVehicle = 'right_25';
  end
end

subscribedToTestVeh = 0;
contextSubsToTestVeh = 0;
trackingTestVeh = 0;

vehicleCommandsTested = 0;
edgeCommandsTested = 0;
guiCommandsTested = 0;
indLoopCommandsTested = 0;
junctionCommandsTested = 0;
laneareaCommandsTested = 0;
laneCommandsTested = 0;
muiCommandsTested = 0;
simCommandsTested = 0;
tlsCommandsTested = 0;

getSubscriptionResultsTested = 0;

% Define the traffic light phases in a sumo-readable way
NSGREEN = 'GrGr';
NSYELLOW = 'yryr';
WEGREEN = 'rGrG';
WEYELLOW = 'ryry';

% Define the traffic light program. The duration of the phases is based on
% the time required for a car to cross the traffic light from north to
% south
PROGRAM = {WEYELLOW,WEYELLOW,WEYELLOW,NSGREEN,NSGREEN,NSGREEN,NSGREEN,NSGREEN,NSGREEN,NSGREEN,NSGREEN,NSYELLOW,NSYELLOW,WEGREEN};

programPointer = length(PROGRAM);
%steps = zeros(1,800);

%% GETIDLIST COMMANDS
% lanearea_dets = traci.lanearea.getIDList();
% fprintf('IDs of the lanearea detectors in the simulation:\n')
% for i=1:length(lanearea_dets)
%  fprintf('%s\n',lanearea_dets{i});
% end
% 
% edges = traci.edge.getIDList();
% fprintf('IDs of the edges in the simulation:\n')
% for i=1:length(edges)
%  fprintf('%s\n',edges{i});
% end
% 
% views = traci.gui.getIDList();
% fprintf('IDs of the views in the simulation:\n')
% for i=1:length(views)
%  fprintf('%s\n',views{i});
% end
% 
% inductionloops = traci.inductionloop.getIDList();
% fprintf('IDs of the induction loops in the simulation:\n')
% for i=1:length(inductionloops)
%  fprintf('%s\n',inductionloops{i});
% end
% 
% junctions = traci.junction.getIDList();
% fprintf('IDs of the junctions in the simulation:\n')
% for i=1:length(junctions)
%  fprintf('%s\n',junctions{i});
% end
% 
% lanes = traci.lane.getIDList();
% fprintf('IDs of the lanes in the simulation:\n')
% for i=1:length(lanes)
%  fprintf('%s\n',lanes{i});
% end
% 
% detectors = traci.multientryexit.getIDList();
% fprintf('IDs of the multi-entry/multi-exit detectors in the simulation:\n')
% for i=1:length(detectors)
%  fprintf('%s\n',detectors{i});
% end
% 
% polygons = traci.polygon.getIDList();
% fprintf('IDs of te polygons in the simulation:\n')
% for i=1:length(polygons)
%  fprintf('%s\n',polygons{i});
% end
% 
% routes = traci.route.getIDList();
% fprintf('IDs of the routes in the simulation:\n')
% for i=1:length(routes)
%  fprintf('%s\n',routes{i});
% end
% 
% trafficlights = traci.trafficlights.getIDList();
% fprintf('IDs of the traffic lights in the simulation:\n')
% for i=1:length(trafficlights)
%  fprintf('%s\n',trafficlights{i});
% end
% 
% vehicletypes = traci.vehicletype.getIDList();
% fprintf('IDs of the vehicle types in the simulation:\n')
% for i=1:length(vehicletypes)
%  fprintf('%s\n',vehicletypes{i});
% end

% THE GETIDLIST COMMAND FOR POIS AND POLYGONS IS TESTED AFTER ADDING THOSE
% OBJECTS TO THE SIMULATION
%
% THE GETIDLIST COMMAND FOR VEHICLES IS PERFOMED ONCE THE VEHICLES ARE
% LOADED IN THE NETWORK, IN THE MAIN LOOP


%% SUBSCIBE COMMANDS: Note that you have to create the required detectors in the cross.det.xml file
% traci.edge.subscribe('1i');
% traci.edge.subscribe('1i', {traci.constants.LAST_STEP_VEHICLE_NUMBER,...
%  traci.constants.LAST_STEP_VEHICLE_ID_LIST});
% traci.gui.subscribe('View #0');
% traci.inductionloop.subscribe('0');
% traci.junction.subscribe('0');
% traci.lane.subscribe('1i_0',{traci.constants.VAR_WAITING_TIME});
% traci.lanearea.subscribe('0', {traci.constants.JAM_LENGTH_METERS});
% traci.multientryexit.subscribe('0', {traci.constants.LAST_STEP_MEAN_SPEED});
% traci.route.subscribe('down');
% traci.simulation.subscribe();
% traci.trafficlights.subscribe('0',{traci.constants.TL_RED_YELLOW_GREEN_STATE});
% traci.vehicletype.subscribe('typeWE');

%% POI AND POLYGON COMMANDS
% traci.poi.add('mypoi', 550, 550, [255 255 0 0], '', 1);
% pois = traci.poi.getIDList()
% fprintf('IDs of the pois in the simulation:\n')
% for i=1:length(pois)
%  fprintf('%s\n',pois{i});
% end
% traci.poi.subscribe('mypoi');
% poiColor = traci.poi.getColor('mypoi')
% traci.poi.setType('mypoi', 'mypoitype');
% poitype = traci.poi.getType('mypoi')
% traci.poi.setPosition('mypoi', 550, 580);
% poiposition = traci.poi.getPosition('mypoi')
% traci.poi.setColor('mypoi', [255 255 255 255]);
% %traci.poi.remove('mypoi', 1);
% 
% traci.polygon.add('mypolygon', {[440,440],[440,450],[450,440],[450,450]},...
%  [0 255 255 255], false, '', 1, 1);
% traci.polygon.add('my2ndpolygon', {[400,400],[400,420],[430,420],[430,400],[400,400]},...
%  [255 0 0 255], true, '', 1);
% polygons = traci.polygon.getIDList()
% fprintf('IDs of the polygons in the simulation:\n')
% for i=1:length(polygons)
%  fprintf('%s\n',polygons{i});
% end
% traci.polygon.subscribe('mypolygon');
% polygonColor = traci.polygon.getColor('mypolygon')
% traci.polygon.setType('mypolygon', 'mypolygontype');
% polygontype = traci.polygon.getType('mypolygon')
% traci.polygon.setShape('mypolygon', {[400,400],[400,420],[430,420],[430,400],[400,400]});
% polygonshape = traci.polygon.getShape('mypolygon')
% traci.polygon.setColor('mypolygon', [255 255 255 255]);
% traci.polygon.setLineWidth('mypolygon', 1.5);
% traci.polygon.getLineWidth('mypolygon')
% traci.polygon.setFilled('mypolygon', true);
% traci.polygon.getFilled('mypolygon')
%traci.polygon.remove('mypolygon', 1);

%% CONTEXT SUBSCRIPTIONS
% It's worth noting that, according to
% http://sumo-sim.org/userdoc/TraCI/Object_Context_Subscription.html, only
% the following SUMO objects are supported: inductive loops, lanes,
% vehicles, points-of-interest, polygons, junctions, edges.

% traci.edge.subscribeContext('4i',traci.constants.CMD_GET_LANE_VARIABLE,50);
% traci.inductionloop.subscribeContext('0',traci.constants.CMD_GET_LANE_VARIABLE,20);
% traci.junction.subscribeContext('0',traci.constants.CMD_GET_VEHICLE_VARIABLE,20);
% traci.lane.subscribeContext('4i_0',traci.constants.CMD_GET_LANE_VARIABLE,50);
% traci.poi.subscribeContext('mypoi',traci.constants.CMD_GET_VEHICLE_VARIABLE,50);
% traci.polygon.subscribeContext('mypolygon',traci.constants.CMD_GET_POLYGON_VARIABLE,30);
 
%WElaneoccupancy = zeros(1,800);
%NSlaneoccupancy = zeros(1,800);

% getminexectednumber returns the number of expected vehicles to leave the
% network, which will be the condition to execute the simulation.
%minExpectedNumber = traci.simulation.getMinExpectedNumber();

%% GUI SET COMMANDS
traci.gui.setZoom('View #0', 1000);
traci.gui.setOffset('View #0',  523.7211,  525.9342);
traci.gui.setSchema('View #0',  'real world');
% traci.gui.setBoundary('View #0', 386.95, 485.88, 651.64, 589.01);

%% LANE SET COMMANDS
% traci.lane.setAllowed('1i_0',{'passenger'});
% traci.lane.setDisallowed('1i_0',{'bicycle'});
% traci.lane.setMaxSpeed('1i_0',5);
% traci.lane.setLength('1i_0',450);

%% ROUTE COMMANDS
% traci.route.add('up',{'53o','3i','4o','54i'});
% routeUpEdges = traci.route.getEdges('up')

%% VEHICLE TYPE COMMANDS
% typeLength = traci.vehicletype.getLength('typeWE')
% typeMaxSpeed = traci.vehicletype.getMaxSpeed('typeWE')
% typeSpeedFactor = traci.vehicletype.getSpeedFactor('typeWE')
% typeSpeedDeviation = traci.vehicletype.getSpeedDeviation('typeWE')
% typeAccel = traci.vehicletype.getAccel('typeWE')
% typeDecel = traci.vehicletype.getDecel('typeWE')
% typeEmergencyDecel = traci.vehicletype.getEmergencyDecel('typeWE')
% apparentDecel = traci.vehicletype.getApparentDecel('typeWE')
% actionStepLength = traci.vehicletype.getActionStepLength('typeWE')
% typeImperfection = traci.vehicletype.getImperfection('typeWE')
% typeTau = traci.vehicletype.getTau('typeWE')
% typeClass = traci.vehicletype.getVehicleClass('typeWE')
% typeEmissionClass = traci.vehicletype.getEmissionClass('typeWE')
% typeShapeClass = traci.vehicletype.getShapeClass('typeWE')
% typeMinGap = traci.vehicletype.getMinGap('typeWE')
% typeWidth = traci.vehicletype.getWidth('typeWE')
% typeHeight = traci.vehicletype.getHeight('typeWE')
% typeColor = traci.vehicletype.getColor('typeWE')
% typeMaxSpeedLat = traci.vehicletype.getMaxSpeedLat('typeWE')
% typeLateralAlignment = traci.vehicletype.getLateralAlignment('typeWE')
% typeMinGapLat = traci.vehicletype.getMinGapLat('typeWE')
% 
% traci.vehicletype.setLength('typeWE',8);
% traci.vehicletype.setMaxSpeed('typeWE',10);
% traci.vehicletype.setVehicleClass('typeWE','passenger');
% traci.vehicletype.setSpeedFactor('typeWE',0.8);
% traci.vehicletype.setSpeedDeviation('typeWE',0.2);
% traci.vehicletype.setEmissionClass('typeWE','unknown');
% traci.vehicletype.setWidth('typeWE',1);
% traci.vehicletype.setHeight('typeWE', 3);
% traci.vehicletype.setMinGap('typeWE',1);
% traci.vehicletype.setMinGapLat('typeWE', 0.5);
% traci.vehicletype.setMaxSpeedLat('typeWE', 0.4);
% traci.vehicletype.setLateralAlignment('typeWE', 'left');
% traci.vehicletype.setShapeClass('typeWE','');
% traci.vehicletype.setAccel('typeWE',5);
% traci.vehicletype.setDecel('typeWE',3);
% traci.vehicletype.setEmergencyDecel('typeWE', 5);
% traci.vehicletype.setActionStepLength('typeWE', 2)
% traci.vehicletype.setApparentDecel('typeWE', 3)
% traci.vehicletype.setImperfection('typeWE',0.6);
% traci.vehicletype.setTau('typeWE',0.1);
% traci.vehicletype.setColor('typeWE',[255 255 255 255]);
% traci.vehicletype.copy('typeWE', 'typeWE_copy');

step = 1;
%for i=1:length(steps)
while traci.simulation.getMinExpectedNumber() > 0
  % Here, we demonstrate how to use the simulationStep command using an
  % argument. In this case, the simulation is performed each 5 seconds,
  % note the behavior when you increase the delay in the gui
%  traci.simulationStep(5*step);
%  pause(1);
%   traci.simulationStep();
  traci.simulation.step();
%   traci.simulation.step(5*step);
  programPointer = min(programPointer+1, length(PROGRAM));
    
  % Get the number of vehicles that passed through the induction loop in
  % the last simulation step
  numPriorityVehicles = traci.inductionloop.getLastStepVehicleNumber('0');
    
  % SHOW THE VEHICLES IDS INSIDE THE NETWORK
%  if step == 100
%    vehicles = traci.vehicle.getIDList();
%    fprintf('IDs of the vehicles in the simulation\n')
%    for j=1:length(vehicles)
%      fprintf('%s\n',vehicles{j});
%    end
%  end
    
  % VEHICLE COMMANDS
%   if ismember(testVehicle, vehicles)      
%     if ~vehicleCommandsTested
     %% VEHICLE GET COMMANDS
%      traci.vehicle.setRoute(testVehicle, {'51o' '1i' '2o'});
     
%      vehSpeed = traci.vehicle.getSpeed(testVehicle)
%      vehAcceleration = traci.vehicle.getAcceleration(testVehicle)
%      vehSpeedWOTraci = traci.vehicle.getSpeedWithoutTraCI(testVehicle)
%      vehPosition = traci.vehicle.getPosition(testVehicle)
%      vehPosition3D = traci.vehicle.getPosition3D(testVehicle)
%      vehAngle = traci.vehicle.getAngle(testVehicle)
%      vehRoadID = traci.vehicle.getRoadID(testVehicle)
%      vehLaneID = traci.vehicle.getLaneID(testVehicle)
%      vehLaneIndex = traci.vehicle.getLaneIndex(testVehicle)
%      vehTypeID = traci.vehicle.getTypeID(testVehicle)
%      vehRouteID = traci.vehicle.getRouteID(testVehicle)
%      vehRouteIndex = traci.vehicle.getRouteIndex(testVehicle)
%      vehRoute = traci.vehicle.getRoute(testVehicle)
%      vehLanePos = traci.vehicle.getLanePosition(testVehicle)
%      vehColor = traci.vehicle.getColor(testVehicle)
%      vehCO2Emission = traci.vehicle.getCO2Emission(testVehicle)
%      vehCOEmission = traci.vehicle.getCOEmission(testVehicle)
%      vehHCEmission = traci.vehicle.getHCEmission(testVehicle)
%      vehPmxEmission = traci.vehicle.getPMxEmission(testVehicle)
%      vehNOxEmission = traci.vehicle.getNOxEmission(testVehicle)
%      vehFuelConsumption = traci.vehicle.getFuelConsumption(testVehicle)
%      vehNoiseEmission = traci.vehicle.getNoiseEmission(testVehicle)
%      vehElectricityConsumption = traci.vehicle.getElectricityConsumption(testVehicle)
%      vehPersonNumber = traci.vehicle.getPersonNumber(testVehicle)
%      vehPersonIDs = traci.vehicle.getPersonIDList(testVehicle)
%      vehAdaptedTraveltime = traci.vehicle.getAdaptedTraveltime(testVehicle,10,'1i')
%      vehEffort = traci.vehicle.getEffort(testVehicle,10,'1i')
%      vehValidRoute = traci.vehicle.isRouteValid(testVehicle)
%      vehSignals = traci.vehicle.getSignals(testVehicle)
%      vehLength = traci.vehicle.getLength(testVehicle)
%      vehMaxSpeed = traci.vehicle.getMaxSpeed(testVehicle)
%      vehLateralLanePos = traci.vehicle.getLateralLanePosition(testVehicle)
%      vehMaxSpeedLat = traci.vehicle.getMaxSpeedLat(testVehicle)
%      vehLatAlignment = traci.vehicle.getLateralAlignment(testVehicle)
%      vehMinGapLat = traci.vehicle.getMinGapLat(testVehicle)
%      vehAllowedSpeed = traci.vehicle.getAllowedSpeed(testVehicle)
%      vehClass = traci.vehicle.getVehicleClass(testVehicle)
%      vehSpeedFactor = traci.vehicle.getSpeedFactor(testVehicle)
%      vehSpeedDeviation = traci.vehicle.getSpeedDeviation(testVehicle)
%      vehEmissionClass = traci.vehicle.getEmissionClass(testVehicle)
%      vehWaitingTime = traci.vehicle.getWaitingTime(testVehicle)
%      vehAccumWaitingTime = traci.vehicle.getAccumulatedWaitingTime(testVehicle)
%      lcMode = traci.vehicle.getLaneChangeMode(testVehicle);
%      vehSpeedMode = traci.vehicle.getSpeedMode(testVehicle)
%      vehSlope = traci.vehicle.getSlope(testVehicle)
%      vehWidth = traci.vehicle.getWidth(testVehicle)
%      vehHeight = traci.vehicle.getHeight(testVehicle)
%      vehLine = traci.vehicle.getLine(testVehicle)
%      vehVia = traci.vehicle.getVia(testVehicle)
%      vehMinGap = traci.vehicle.getMinGap(testVehicle)
%      vehShapeClass = traci.vehicle.getShapeClass(testVehicle)
%      vehAccel = traci.vehicle.getAccel(testVehicle)
%      vehDecel = traci.vehicle.getDecel(testVehicle)
%      vehEmergencyDecel = traci.vehicle.getEmergencyDecel(testVehicle)
%      vehApparentDecel = traci.vehicle.getApparentDecel(testVehicle)
%      actionStepLength = traci.vehicle.getActionStepLength(testVehicle)
%      lastActionTime = traci.vehicle.getLastActionTime(testVehicle)
%      vehImperfection = traci.vehicle.getImperfection(testVehicle)
%      vehTau = traci.vehicle.getTau(testVehicle)
%      bestLanes = traci.vehicle.getBestLanes(testVehicle)
%      vehLeader = traci.vehicle.getLeader(testVehicle, 1)
%      vehNextTLS = traci.vehicle.getNextTLS(testVehicle)
%      nextStops = traci.vehicle.getNextStops(testVehicle)
%      vehDrivingDistance = traci.vehicle.getDrivingDistance(testVehicle,'2o',30)
%      vehDrivingDistance2D = traci.vehicle.getDrivingDistance2D(testVehicle,620,510)
%      vehDistance = traci.vehicle.getDistance(testVehicle)
%      vehStopState = traci.vehicle.getStopState(testVehicle)
%      vehIsStopped = traci.vehicle.isStopped(testVehicle)
%      vehIsStoppedParking = traci.vehicle.isStoppedParking(testVehicle)
%      vehIsStoppedTriggered = traci.vehicle.isStoppedTriggered(testVehicle)
%      vehIsAtBusStop = traci.vehicle.isAtBusStop(testVehicle)
%      vehIsAtContainerStop = traci.vehicle.isAtContainerStop(testVehicle)
%      lcState = traci.vehicle.getLaneChangeState(testVehicle, 1)
%      vehCouldChangeLane = traci.vehicle.couldChangeLane(testVehicle, 1)
%      vehWantsAndCouldChangeLane = traci.vehicle.wantsAndCouldChangeLane(testVehicle, 1)
%      routingMode = traci.vehicle.getRoutingMode(testVehicle)
     
     %% VEHICLE SET COMMANDS
%      traci.vehicle.add('myvehicle', 'down');
%      traci.gui.trackVehicle('View #0', 'myvehicle');
%      traci.vehicle.remove('myvehicle');
%      traci.vehicle.setMaxSpeed(testVehicle, 5);
%      traci.vehicle.setMaxSpeedLat(testVehicle, 2);
%      traci.vehicle.setStop(testVehicle, '1i', 50, 0, 40);
% %      traci.vehicle.setBusStop(testVehicle, 'busStop1');
%      traci.vehicle.changeLane(testVehicle, 0, 40);
%      traci.vehicle.changeLaneRelative(testVehicle, 0, 40);
%      traci.vehicle.changeSublane(testVehicle, 1.2);
%      traci.vehicle.slowDown(testVehicle, 1, 180);
%      traci.vehicle.openGap(testVehicle, 2, 1, 20, 1);
%      traci.vehicle.deactivateGapControl(testVehicle);
%      traci.vehicle.changeTarget(testVehicle, '2o');
%      traci.vehicle.setType(testVehicle, 'typeWE');
%      traci.vehicle.setRouteID(testVehicle, 'right');
%      traci.vehicle.setRoute(testVehicle, {'51o','1i','2o','52i'});
%      traci.vehicle.updateBestLanes(testVehicle);
%      traci.vehicle.setAdaptedTraveltime(testVehicle, '1i');
%      traci.vehicle.setEffort(testVehicle, '1i');
%      traci.vehicle.setRoutingMode(testVehicle,...
%          uint8(sscanf(traci.constants.ROUTING_MODE_DEFAULT,'%x')));
%      traci.vehicle.rerouteTraveltime(testVehicle);
%      traci.vehicle.rerouteEffort(testVehicle);
%      traci.vehicle.setSignals(testVehicle, 2);
%      traci.vehicle.moveTo(testVehicle,'1i_0',20);
%      traci.vehicle.setSpeed(testVehicle, 13.8999);
%      traci.vehicle.setSpeed(testVehicle, 12.0001);
%      traci.vehicle.setColor(testVehicle,[0 0 255 255]);
%      traci.vehicle.setLength(testVehicle, 10);
%      traci.vehicle.setVehicleClass(testVehicle, 'passenger');
%      traci.vehicle.setSpeedFactor(testVehicle, 0.6);
%      traci.vehicle.setEmissionClass(testVehicle, 'unknown');
%      traci.vehicle.setWidth(testVehicle, 3);
%      traci.vehicle.setHeight(testVehicle, 2);
%      traci.vehicle.setLine(testVehicle, 'line1');
%      traci.vehicle.setVia(testVehicle, {'2o'});
%      traci.vehicle.setMinGap(testVehicle, 10);
%      traci.vehicle.setMinGapLat(testVehicle, 2);
%      traci.vehicle.setLateralAlignment(testVehicle, 'right');
%      traci.vehicle.setShapeClass(testVehicle, '');
%      traci.vehicle.setAccel(testVehicle, 2);
%      traci.vehicle.setDecel(testVehicle, 5);
%      traci.vehicle.setEmergencyDecel(testVehicle, 9);
%      traci.vehicle.setApparentDecel(testVehicle, 4.5);
%      traci.vehicle.setActionStepLength(testVehicle, 1);
%      traci.vehicle.setImperfection(testVehicle, 1);
%      traci.vehicle.setTau(testVehicle, 1);
%      traci.vehicle.setLaneChangeMode(testVehicle, 0);
%      traci.vehicle.setSpeedMode(testVehicle, 31);
%      traci.vehicle.moveToXY('right_10','3i',0,1000,508.5,90);
%        
%      if step == 100
%        testVehDecel = traci.vehicle.getDecel(testVehicle)
%        traci.vehicle.setDecel(testVehicle, 5);
%      end
       
%      if step > 100
%        testVehDecel = traci.vehicle.getDecel(testVehicle)
%      end
       
%      vehicleCommandsTested = 1;
       
  % Subscribe to the vehicle with the id contained in the variable "testVehicle" 
	% when it is loaded in the network       
%    if ~subscribedToTestVeh
%      traci.vehicle.subscribe(testVehicle);
%      subscribedToTestVeh = 1;
%      vehicleCommandsTested = 1;
%    end
       
%    if ~contextSubsToTestVeh
%      traci.vehicle.subscribeContext(testVehicle,...
%        traci.constants.CMD_GET_VEHICLE_VARIABLE, 20,...
%        {'0x40', '0x42','0x43','0x5b','0x51','0x56','0x7a'});
%      traci.vehicle.subscribeContext(testVehicle,...
%        traci.constants.CMD_GET_VEHICLE_VARIABLE, 10, {constants.VAR_WAITING_TIME});
%      traci.vehicle.subscribeContext(testVehicle,...
%        traci.constants.CMD_GET_VEHICLE_VARIABLE, 10);
     
     % TEST VEHICLE SUBSCRIPTION FILTER COMMANDS
%      traci.vehicle.addSubscriptionFilterNoOpposite();
%      traci.vehicle.addSubscriptionFilterDownstreamDistance(10);
%      traci.vehicle.addSubscriptionFilterUpstreamDistance(10);
%      traci.vehicle.addSubscriptionFilterLanes([0], 1, 10, 10);
%      traci.vehicle.addSubscriptionFilterCFManeuver();
%      traci.vehicle.addSubscriptionFilterLCManeuver(1);
%      traci.vehicle.addSubscriptionFilterTurn();
%      traci.vehicle.addSubscriptionFilterVClass({'passenger'});
%      traci.vehicle.addSubscriptionFilterVType({'typeWE'});
    
%      contextSubsToTestVeh = 1;
%      vehicleCommandsTested = 1;
%    end
   
%           
%    testVehicleRoad = char(traci.vehicle.getSubscriptionResults(testVehicle).get(...
%      traci.constants.VAR_ROAD_ID))
%    testVehiclePos = traci.vehicle.getSubscriptionResults(testVehicle).get(...
%      traci.constants.VAR_LANEPOSITION)
%        
%    if ~trackingTestVeh
%      traci.gui.trackVehicle('View #0', testVehicle);
%      trackingTestVeh = 1;
%    end
%     end
%   end
    
  %% GETSUBSCRIPTIONRESULTS COMMANDS: Note that you have to create the required detectors in the cross.det.xml file
%   if ~getSubscriptionResultsTested && step == 100
%       edge1iSubsResults = traci.edge.getSubscriptionResults('1i');
%       edge1iVehNumber = edge1iSubsResults(traci.constants.LAST_STEP_VEHICLE_NUMBER)
%       edge1iVehIDs = char(edge1iSubsResults(traci.constants.LAST_STEP_VEHICLE_ID_LIST))
%       guiSubsResults = traci.gui.getSubscriptionResults('View #0');
%       offset = guiSubsResults(traci.constants.VAR_VIEW_OFFSET)
%       indloopSubsResults = traci.inductionloop.getSubscriptionResults('0');
%       indloopVehNumber = indloopSubsResults(traci.constants.LAST_STEP_VEHICLE_NUMBER)
%       junctionSubsResults = traci.junction.getSubscriptionResults('0');
%       junctionPosition = junctionSubsResults( traci.constants.VAR_POSITION)
%       laneSubsResults = traci.lane.getSubscriptionResults('1i_0');
%       laneWaitingTime = laneSubsResults(traci.constants.VAR_WAITING_TIME)
%       laneareaSubsResults = traci.lanearea.getSubscriptionResults('0');
%       laneAreaJamLength = laneareaSubsResults(traci.constants.JAM_LENGTH_METERS)
%       multiEntryExitSubsResults = traci.multientryexit.getSubscriptionResults('0');
%       meanSpeedLane2 = multiEntryExitSubsResults(traci.constants.LAST_STEP_MEAN_SPEED)
%       poiSubsResults = traci.poi.getSubscriptionResults('mypoi');
%       poiPosition = poiSubsResults(traci.constants.VAR_POSITION)
%       polygonSubsResults = traci.polygon.getSubscriptionResults('mypolygon');
%       polygonPosition = polygonSubsResults(traci.constants.VAR_SHAPE)
%       routeSubsResults = traci.route.getSubscriptionResults('down');
%       routeList = char(routeSubsResults(traci.constants.ID_LIST))
%       simSubsResults = traci.simulation.getSubscriptionResults();
%       departedVehicleIDs = char(simSubsResults(traci.constants.VAR_DEPARTED_VEHICLES_IDS))
%       trafficLightsSubsResults = traci.trafficlights.getSubscriptionResults('0');
%       tlsCurrentPhase = trafficLightsSubsResults(traci.constants.TL_RED_YELLOW_GREEN_STATE)
%       vehTypeSubsResults = traci.vehicletype.getSubscriptionResults('typeWE');
%       maxSpeedWE = vehTypeSubsResults(traci.constants.VAR_MAXSPEED)
%       getSubscriptionResultsTested = 1;
%   end
%   if subscribedToTestVeh
%       vehSubscriptionResults = traci.vehicle.getSubscriptionResults(testVehicle);
%       vehRoadID = vehSubscriptionResults(traci.constants.VAR_ROAD_ID)
%       vehLanePos = vehSubscriptionResults(traci.constants.VAR_LANEPOSITION)
%   end
    
    %% GET CONTEXT SUBSCRIPTION RESULTS COMMANDS
%     edge4i0ContextResults = traci.edge.getContextSubscriptionResults('4i');
%     occupancy4i0Handle1 = edge4i0ContextResults('4i_0');
%     occupancy4i0 = occupancy4i0Handle1(traci.constants.LAST_STEP_VEHICLE_NUMBER);
%     fprintf('%d\n',occupancy4i0);
%     
%     loop0ContextResults = traci.inductionloop.getContextSubscriptionResults('0');
%     priorityVehiclesPassedHandle = loop0ContextResults('4i_0');
%     priorityVehiclesPassed = priorityVehiclesPassedHandle(traci.constants.LAST_STEP_VEHICLE_NUMBER);
%     fprintf('%d\n',priorityVehiclesPassed);
%     
%     junctionContextResults = traci.junction.getContextSubscriptionResults('0');
%     
%     laneContextSubscriptionResults = traci.lane.getContextSubscriptionResults('4i_0');
%     poiContextSubscriptionResults = traci.poi.getContextSubscriptionResults('mypoi');
%     polygonContextSubscriptionResults = traci.polygon.getContextSubscriptionResults('mypolygon');
%    if contextSubsToTestVeh
%        vehicleContextSubscriptionResults = traci.vehicle.getContextSubscriptionResults(testVehicle);
%        if ~strcmp(vehicleContextSubscriptionResults,'None')
% %            testVehicleSubsResults = vehicleContextSubscriptionResults(testVehicle);
% %            testVehicleWaitingTime = testVehicleSubsResults(traci.constants.VAR_WAITING_TIME)
% %            testVehicleLanePos = testVehicleSubsResults(traci.constants.VAR_LANEPOSITION)
%              vehicleContextSubscriptionResults
%        end
%    end
    
	%% LANE AREA DETECTOR COMMANDS: Note that you have to create the detector in the cross.det.xml file
%  if ~laneareaCommandsTested
%    laneareaDetectorIDCount = traci.lanearea.getIDCount();
%    fprintf('Number of lanearea detectors in the simulation: %d\n',...
%      laneareaDetectorIDCount);
%    
%    JamLengthVehicle = traci.lanearea.getJamLengthVehicle('0');
%    fprintf('Jam lenght in vehicles in the lanearea detector 0: %d\n',...
%      JamLengthVehicle);
%    
%    JamLengthMeters = traci.lanearea.getJamLengthMeters('0');
%    fprintf('Jam lenght in meters in the lanearea detector 0: %d\n',...
%      JamLengthMeters);
%    
%    vehicleMeanSpeedlanearea0 = traci.lanearea.getLastStepMeanSpeed('0');
%    fprintf('Average speed in the lanearea detector 0: %d\n',...
%      vehicleMeanSpeedlanearea0);
%    
%    vehicleOccupancylanearea0 = traci.lanearea.getLastStepOccupancy('0');
%    fprintf('Occupancy in the lanearea detector 0 1i: %d\n',...
%      vehicleOccupancylanearea0);
%    
%    laneareaCommandsTested = 1;
%  end
	
  %% EDGE COMMANDS 
%  if ~edgeCommandsTested
%    edgeIDCount = traci.edge.getIDCount();
%    fprintf('Number of edges in the simulation: %d\n',edgeIDCount);
%      
%    travelTime = traci.edge.getAdaptedTraveltime('1i',10);
%    fprintf('Travel time in 10 seconds in the edge 1i: %d\n',travelTime);
%    
%    waitingTime = traci.edge.getWaitingTime('1i');
%    fprintf('Waiting time in the edge 1i: %d\n',waitingTime);
%      
%    effort = traci.edge.getEffort('1i',10);
%    fprintf('Travel effort in 10 seconds in the edge 1i: %d\n',effort);
%     
%    CO2EmissionEdge1i = traci.edge.getCO2Emission('1i');
%    fprintf('CO2 emission in the edge 1i: %d\n',CO2EmissionEdge1i);
%      
%    COEmissionEdge1i = traci.edge.getCOEmission('1i');
%    fprintf('CO emission in the edge 1i: %d\n',COEmissionEdge1i);
%      
%    HCEmissionEdge1i = traci.edge.getHCEmission('1i');
%    fprintf('HC emission in the edge 1i: %d\n',HCEmissionEdge1i);
%      
%    PMxEmissionEdge1i = traci.edge.getPmxEmission('1i');
%    fprintf('PMx emission in the edge 1i: %d\n',PMxEmissionEdge1i);
%      
%    NOxEmissionEdge1i = traci.edge.getNOxEmission('1i');
%    fprintf('NOx emission in the edge 1i: %d\n',NOxEmissionEdge1i);
%      
%    fuelConsumptionEdge1i = traci.edge.getFuelConsumption('1i');
%    fprintf('Fuel consumption in the edge 1i: %d\n',fuelConsumptionEdge1i);
%      
%    noiseEmissionEdge1i = traci.edge.getNoiseEmission('1i');
%    fprintf('Noise emission in the edge 1i: %d\n',noiseEmissionEdge1i);
%    
%    electricityEdge1i = traci.edge.getElectricityConsumption('1i');
%    fprintf('Electricity consumption in the edge 1i: %d\n',electricityEdge1i);
%      
%    vehicleMeanSpeedEdge1i = traci.edge.getLastStepMeanSpeed('1i');
%    fprintf('Average speed in the edge 1i: %d\n',vehicleMeanSpeedEdge1i);
%      
%    vehicleOccupancyEdge1i = traci.edge.getLastStepOccupancy('1i');
%    fprintf('Occupancy in the edge 1i: %d\n',vehicleOccupancyEdge1i);
%      
%    vehicleMeanLengthEdge1i = traci.edge.getLastStepLength('1i');
%    fprintf('Average length in the edge 1i: %d\n',vehicleMeanLengthEdge1i);
%      
%    vehicleTravelTimeEdge1i = traci.edge.getTraveltime('1i');
%    fprintf('Average time of the vehicles in the edge 1i: %d\n',vehicleTravelTimeEdge1i);
%      
%    vehicleHaltingEdge1i = traci.edge.getLastStepHaltingNumber('1i');
%    fprintf('Stopped vehicles in the edge 1i: %d\n',vehicleHaltingEdge1i);
%      
%    vehicleIDsEdge1i = traci.edge.getLastStepVehicleIDs('1i');
%    fprintf('IDs of the vehicles in the edge 1i: \n');
%    disp(vehicleIDsEdge1i)
%    
%    streetNameEdge1i = traci.edge.getStreetName('1i');
%    fprintf('Street name of the edge 1i: \n');
%    disp(streetNameEdge1i)
%    
%    laneNumberEdge1i = traci.edge.getLaneNumber('1i');
%    fprintf('Number of lanes of the edge 1i: \n');
%    disp(laneNumberEdge1i)
%      
%    traci.edge.adaptTraveltime('1i',15);
%     
%    traci.edge.setEffort('1i',1.343);
%      
%    traci.edge.setMaxSpeed('1i',5);
%    
%    edgeCommandsTested = 1;
%  end
    
  %% GUI COMMANDS
%  if ~guiCommandsTested
%    guizoom = traci.gui.getZoom()
%    offset = traci.gui.getOffset()
%    schema = traci.gui.getSchema()
%    boundary = traci.gui.getBoundary()
%    hasDefaultView = traci.gui.hasView('default')
%    guiCommandsTested = 1;
%  end
    
  %% INDUCTION LOOP COMMANDS
%  if ~indLoopCommandsTested
%    loop0position = traci.inductionloop.getPosition('0');
%    loop0LaneID = traci.inductionloop.getLaneID('0')
%    loop0MeanSpeed = traci.inductionloop.getLastStepMeanSpeed('0')
%    loop0VehicleIDs = traci.inductionloop.getLastStepVehicleIDs('0')
%    loop0Occupancy = traci.inductionloop.getLastStepOccupancy('0')
%    loop0MeanLength = traci.inductionloop.getLastStepMeanLength('0')
%    loop0TimeSinceDetection = traci.inductionloop.getTimeSinceDetection('0')
%    indLoopCommandsTested = 1;
%  end
    
  %% JUNCTION COMMANDS
%  if ~junctionCommandsTested
%    junctionPosition = traci.junction.getPosition('0')
%    junctionShape = traci.junction.getShape('0')
%    junctionCommandsTested = 1;
%  end
    
  %% LANE GET COMMANDS
%  if ~laneCommandsTested
%    lane1i0Length = traci.lane.getLength('1i_0')
%    lane1i0MaxSpeed = traci.lane.getMaxSpeed('1i_0')
%    lane1i0Width = traci.lane.getWidth('1i_0')
%    lane1i0AllowedVehicles = traci.lane.getAllowed('1i_0')
%    lane1i0DisallowedVehicles = traci.lane.getDisallowed('1i_0')
%    lane1i0LinkNumber = traci.lane.getLinkNumber('1i_0')
%    lane1i0Links = traci.lane.getLinks('1i_0')
%    lane1i0Shape = traci.lane.getShape('1i_0')
%    lane1i0EdgeID = traci.lane.getEdgeID('1i_0')
%    lane1i0CO2Emmision = traci.lane.getCO2Emission('1i_0')
%    lane1i0COEmmision = traci.lane.getCOEmission('1i_0')
%    lane1i0HCEmmision = traci.lane.getHCEmission('1i_0')
%    lane1i0PMxEmmision = traci.lane.getPMxEmission('1i_0')
%    lane1i0NOxEmmision = traci.lane.getNOxEmission('1i_0')
%    lane1i0FuelConsumption = traci.lane.getFuelConsumption('1i_0')
%    lane1i0NoiseEmission = traci.lane.getNoiseEmission('1i_0')
%    lane1i0MeanSpeed = traci.lane.getLastStepMeanSpeed('1i_0')
%    lane1i0Occupancy = traci.lane.getLastStepOccupancy('1i_0')
%    lane1i0MeanVehicleLength = traci.lane.getLastStepLength('1i_0')
%    lane1i0TravelTime = traci.lane.getTraveltime('1i_0')
%    lane1i0HaltingNumber = traci.lane.getLastStepHaltingNumber('1i_0')
%    lane1i0VehicleIDs = traci.lane.getLastStepVehicleIDs('1i_0')
%    lane1i0ElectricityConsumption = traci.lane.getElectricityConsumption('1i_0')
%    lane1i0WaitingTime = traci.lane.getWaitingTime('1i_0')
%    foes = traci.lane.getFoes('1i_0', '2o_0')
%    internalFoes = traci.lane.getInternalFoes('1i_0')
%    laneCommandsTested = 1;
%  end
   
  %% MULTIENTRY=EXIT COMMANDS: Note that you have to create the detector in the cross.det.xml file  
%  if ~muiCommandsTested
%    muiVehicleNumber = traci.multientryexit.getLastStepVehicleNumber('0')
%    muiMeanSpeed = traci.multientryexit.getLastStepMeanSpeed('0')
%    muiVehIDs = traci.multientryexit.getLastStepVehicleIDs('0')
%    muiHaltingVehicles = traci.multientryexit.getLastStepHaltingNumber('0')
%    muiCommandsTested = 1;
%  end
    
  %% SIMULATION COMMANDS
%  if ~simCommandsTested
%    time = traci.simulation.getTime()
%    currentTime = traci.simulation.getCurrentTime()
%    loadedNumber = traci.simulation.getLoadedNumber()
%    loadedIDList = traci.simulation.getLoadedIDList()
%    departedNumber = traci.simulation.getDepartedNumber()
%    departedIDList = traci.simulation.getDepartedIDList()
%    arrivedNumber = traci.simulation.getArrivedNumber()
%    arrivedIDList = traci.simulation.getArrivedIDList()
%    parkingStartingNumber = traci.simulation.getParkingStartingVehiclesNumber()
%    parkingStartingIDs = traci.simulation.getParkingStartingVehiclesIDList()
%    parkingEndingNumber = traci.simulation.getParkingEndingVehiclesNumber()
%    parkingEndingIDs = traci.simulation.getParkingEndingVehiclesIDList()
%    stopStartingNumber = traci.simulation.getStopStartingVehiclesNumber()
%    stopStartingIDs = traci.simulation.getStopStartingVehiclesIDList()
%    stopEndingNumber = traci.simulation.getStopEndingVehiclesNumber()
%    stopEndingIDs = traci.simulation.getStopEndingVehiclesIDList()
%    collidingNumber = traci.simulation.getCollidingVehiclesNumber()
%    collidingIDs = traci.simulation.getCollidingVehiclesIDList()
%    emergencyStoppingNumber = traci.simulation.getEmergencyStoppingVehiclesNumber()
%    emergencyStoppingIDs = traci.simulation.getEmergencyStoppingVehiclesIDList()
%    busStopWaiting = traci.simulation.getBusStopWaiting('busStop1')
%    startingTeleportNumber = traci.simulation.getStartingTeleportNumber()
%    startingTeleportIDList = traci.simulation.getStartingTeleportIDList()
%    endingTeleportNumber = traci.simulation.getEndingTeleportNumber()
%    endingTeleportIDs = traci.simulation.getEndingTeleportIDList()
%    deltaT = traci.simulation.getDeltaT()
%    netBoundary = traci.simulation.getNetBoundary()
%    [x2d,y2d] = traci.simulation.convert2D('1i',10)
%    [x3d,y3d,z3d] = traci.simulation.convert3D('1i',10)
%    [roadID pos laneID] = traci.simulation.convertRoad(20, 508.35)
%    [longitude latitude] = traci.simulation.convertGeo(20, 508.35)
%    distance2D = traci.simulation.getDistance2D(20, 508.35, 30, 508.35)
%    distanceRoad = traci.simulation.getDistanceRoad('1i', 10, '1i', 20)
%    route = traci.simulation.findRoute('1i','2o')
%    interModalRoute = traci.simulation.findIntermodalRoute('1i','2o')
%    simCommandsTested = 1;
%  end
    
  %% TRAFFIC LIGHTS COMMANDS   
%   if ~tlsCommandsTested
%       tlsRYGState = traci.trafficlights.getRedYellowGreenState('0')
%       tlsRYGDefinition = traci.trafficlights.getCompleteRedYellowGreenDefinition('0')
%       tlscontrolledLanes = traci.trafficlights.getControlledLanes('0')
%       tlscontrolledLinks = traci.trafficlights.getControlledLinks('0')
%       tlsProgram = traci.trafficlights.getProgram('0')
%       tlsPhase = traci.trafficlights.getPhase('0')
%       tlsPhaseDuration = traci.trafficlights.getPhaseDuration('0')
%       traci.trafficlights.setPhase('0',0);
%       traci.trafficlights.setProgram('0','0');
%       traci.trafficlights.setPhaseDuration('0',5);
%       myRYGDefinition = traci.trafficlights.Logic('0',0,0,...
%           {traci.trafficlights.Phase(31000,'GrGr',31000,31000),...
%           traci.trafficlights.Phase(31000,'rGrG',31000,31000),...
%           traci.trafficlights.Phase(6000,'ryry',6000,6000)});
%       traci.trafficlights.setCompleteRedYellowGreenDefinition('0',tlsRYGDefinition{1});
%       tlsRYGDefinition = traci.trafficlights.getCompleteRedYellowGreenDefinition('0');
%       tlsCommandsTested = 1;
%       traci.trafficlights.setLinkState('0',0,'o');
%       traci.trafficlights.setLinkState('0',0,'o');
%   end
    
  % Change the phase of the traffic light if a vehicle passed through the
  % induction loop
  if numPriorityVehicles > 0
%    traci.gui.screenshot('View #0','passedvehicle.bmp',-1,-1)
   
%     loop0VehicleData = traci.inductionloop.getVehicleData('0');
    
    if programPointer == length(PROGRAM)
      programPointer = 1;
	elseif ~strcmp(PROGRAM(programPointer), WEYELLOW)
      programPointer = 4;
    end
  end
  traci.trafficlights.setRedYellowGreenState('0', PROGRAM{programPointer});
    
  % AN ADDITIONAL EVIDENCE OF THE TRAFFIC LIGHTS SUBSCRIPTION, DON'T
  % FORGET TO SET THE SUBSCRIPTION BEFORE EXECUTING IT.
  % if no > 0
  %   tlsCurrentPhaseHandle = traci.trafficlights.getSubscriptionResults('0');
  %   tlsCurrentPhase = tlsCurrentPhaseHandle(traci.constants.TL_RED_YELLOW_GREEN_STATE);
  %   fprintf('The traffic lights'' phase changed to: %s\n', tlsCurrentPhase)
  % end
    
  % AN ADDITIONAL EVIDENCE OF THE LANE SUBSCRIPTIONS, ENABLE THE PLOTTING
  % FUNCTIONS BELOW TO VISUALIZE IT.
%  WElaneoccupancy(i) = traci.lane.getLastStepVehicleNumber('1i_0')+...
%  traci.lane.getLastStepVehicleNumber('2i_0');
%  NSlaneoccupancy(i) = traci.lane.getLastStepVehicleNumber('3i_0')+...
%    traci.lane.getLastStepVehicleNumber('4i_0');
     
%  steps(i) = i;
  step = step + 1;
end

traci.close()

%simDurations(simRun) = toc

%end
%
%save('simDurations', 'simDurations');

%stairs(steps, WElaneoccupancy)
%grid on;
%hold;
%stairs(steps, NSlaneoccupancy, '--r')
%legend('WE lane occupancy', 'NS lane occupancy')
%title('Lane occupancy vs time')
%xlabel('t (seconds)')
%ylabel('number of vehicles')
