function setParkingAreaStop(vehID, stopID, varargin)
%setParkingAreaStop Adds or modifies a stop at a parking area with
%the given parameters.
%   setParkingAreaStop(VEHID,STOPID) Sets a stop for the vehicle VEHID
%   in the parking area identified by STOPID for the maximum allowed
%   time.
%   setParkingAreaStop(...,DURATION) Specify the duration of the stop in
%   seconds.
%   setParkingAreaStop(...,UNTIL) Specify the end time of the stop in
%   seconds.
%   setParkingAreaStop(...,FLAGS) Specify the reason of the stop.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Nicolás Arroyo, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setParkingAreaStop.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

p = inputParser;
p.FunctionName = 'vehicle.setParkingAreaStop';
p.addRequired('vehID',@ischar)
p.addRequired('stopID',@ischar)
p.addOptional('duration', 2^31-1, @isnumeric)
p.addOptional('until', -1, @isnumeric)
p.addOptional('flags', constants.STOP_PARKING_AREA, @isnumeric)
p.parse(vehID, stopID, varargin{:})

vehID = p.Results.vehID;
stopID = p.Results.stopID;
duration= p.Results.duration;
until = p.Results.until;
flags = p.Results.flags;

traci.vehicle.setStop(vehID, stopID, duration, until, flags);
