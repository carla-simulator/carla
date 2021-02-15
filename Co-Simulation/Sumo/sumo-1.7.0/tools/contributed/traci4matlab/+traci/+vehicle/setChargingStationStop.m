function setChargingStationStop(vehID, stopID, varargin)
%setChargingStationStop Adds or modifies a stop at a charging station with
%the given parameters.
%   setChargingStationStop(VEHID,STOPID) Sets a stop for the vehicle VEHID
%   in the charging station identified by STOPID for the maximum allowed
%   time.
%   setChargingStationStop(...,DURATION) Specify the duration of the stop in
%   seconds.
%   setChargingStationStop(...,UNTIL) Specify the end time of the stop in
%   seconds.
%   setChargingStationStop(...,FLAGS) Specify the reason of the stop.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Nicolás Arroyo, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setChargingStationStop.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

p = inputParser;
p.FunctionName = 'vehicle.setChargingStationStop';
p.addRequired('vehID',@ischar)
p.addRequired('stopID',@ischar)
p.addOptional('duration', 2^31-1, @isnumeric)
p.addOptional('until', -1, @isnumeric)
p.addOptional('flags', constants.STOP_DEFAULT, @isnumeric)
p.parse(vehID, stopID, varargin{:})

vehID = p.Results.vehID;
stopID = p.Results.stopID;
duration= p.Results.duration;
until = p.Results.until;
flags = p.Results.flags;

traci.vehicle.setStop(vehID, stopID, duration, until, flags);
