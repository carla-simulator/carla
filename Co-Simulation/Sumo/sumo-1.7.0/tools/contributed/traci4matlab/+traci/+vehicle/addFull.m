function addFull(vehID, routeID, varargin)
%addFull Add a vehicle (new style with all possible parameters).
%   addFull(VEHID,ROUTEID) Adds a vehicle in the current time-step with ID 
%   VEHID and assigns the route with ID ROUTEID to it.
%   addFull(...,TYPEID) Specify the type of the vehicle.
%   addFull(...,DEPART) Specify the departure time in seconds.
%   addFull(...,DEPARTLANE) Specify the lane in which the vehicle will start.
%   addFull(...,DEPARTPOS) Specify the position relative to the starting lane.
%   addFull(...,DEPARTSPEED) Specify the starting speed of the vehicle.
%   addFull(...,ARRIVALLANE) Specify the arrival lane of the vehicle.
%   addFull(...,ARRIVALPOS) Specify the arrival position of the vehicle.
%   addFull(...,ARRIVALSPEED) Specify the arrival speed of the vehicle.
%   addFull(...,FROMTAZ) Specify the starting Traffic Assignment Zone.
%   addFull(...,TOTAZ) Specify the arrival Traffic Assignment Zone.
%   addFull(...,LINE) Specify the line of this vehicle.
%   addFull(...,PERSONCAPACITY) Specify the person capacity of the vehicle.
%   addFull(...,PERSONUMBER) Specify the number of persons in the vehicle.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: addFull.m 51 2018-12-30 22:32:29Z afacostag $

traci.vehicle.add(vehID, routeID, varargin);
