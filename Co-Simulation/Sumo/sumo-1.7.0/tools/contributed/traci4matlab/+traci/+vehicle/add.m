function add(vehID, routeID, varargin)
%add Add a vehicle (new style with all possible parameters).
%   add(VEHID,ROUTEID) Adds a vehicle in the current time-step with ID 
%   VEHID and assigns the route with ID ROUTEID to it.
%   add(...,TYPEID) Specify the type of the vehicle.
%   add(...,DEPART) Specify the departure time in seconds.
%   add(...,DEPARTLANE) Specify the lane in which the vehicle will start.
%   add(...,DEPARTPOS) Specify the position relative to the starting lane.
%   add(...,DEPARTSPEED) Specify the starting speed of the vehicle.
%   add(...,ARRIVALLANE) Specify the arrival lane of the vehicle.
%   add(...,ARRIVALPOS) Specify the arrival position of the vehicle.
%   add(...,ARRIVALSPEED) Specify the arrival speed of the vehicle.
%   add(...,FROMTAZ) Specify the starting Traffic Assignment Zone.
%   add(...,TOTAZ) Specify the arrival Traffic Assignment Zone.
%   add(...,LINE) Specify the line of this vehicle.
%   add(...,PERSONCAPACITY) Specify the person capacity of the vehicle.
%   add(...,PERSONUMBER) Specify the number of persons in the vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: add.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants
global message

p = inputParser;
p.FunctionName = 'vehicle.add';
p.addRequired('vehID',@ischar)
p.addRequired('routeID',@ischar)
p.addOptional('typeID', 'DEFAULT_VEHTYPE', @ischar)
p.addOptional('depart', '', @ischar)
p.addOptional('departLane', 'first', @ischar)
p.addOptional('departPos', 'base', @ischar)
p.addOptional('departSpeed', '0', @ischar)
p.addOptional('arrivalLane', 'current', @ischar)
p.addOptional('arrivalPos', 'max', @ischar)
p.addOptional('arrivalSpeed', 'current', @ischar)
p.addOptional('fromTaz', '', @ischar)
p.addOptional('toTaz', '', @ischar)
p.addOptional('line', '', @ischar)
p.addOptional('personCapacity', 0, @isnumeric)
p.addOptional('personNumber', 0, @isnumeric)
p.parse(vehID, routeID, varargin{:})

vehID = p.Results.vehID;
routeID = p.Results.routeID;
typeID = p.Results.typeID;
depart = p.Results.depart;
departLane = p.Results.departLane;
departPos = p.Results.departPos;
departSpeed = p.Results.departSpeed;
arrivalLane = p.Results.arrivalLane;
arrivalPos = p.Results.arrivalPos;
arrivalSpeed = p.Results.arrivalSpeed;
fromTaz = p.Results.fromTaz;
toTaz = p.Results.toTaz;
line = p.Results.line;
personCapacity = p.Results.personCapacity;
personNumber = p.Results.personNumber;

msgString = [uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(14)];

if isempty(depart)
    depart = num2str(traci.simulation.getTime());
end

values = {routeID, typeID, depart, departLane, departPos, departSpeed,...
    arrivalLane, arrivalPos, arrivalSpeed, fromTaz, toTaz, line};

for i = 1:length(values)
    msgString = [msgString uint8(sscanf(constants.TYPE_STRING,'%x')) ...
        traci.packInt32(length(values{i})) uint8(values{i})];
end

msgString = [msgString uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(personCapacity)];
msgString = [msgString uint8(sscanf(constants.TYPE_INTEGER,'%x')) ...
    traci.packInt32(personNumber)];

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.ADD_FULL,...
    vehID, length(msgString));
message.string = [message.string msgString];
traci.sendExact();
