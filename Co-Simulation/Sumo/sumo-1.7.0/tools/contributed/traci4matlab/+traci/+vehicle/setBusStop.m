function setBusStop(vehID, stopID, varargin)
%setStop Set a stop for the vehicle.
%   setBusStop(VEHID,STOPID) Sets a stop for the vehicle VEHID in the bus
%   stop STOPID for the maximum allowed time. The position and the lane index
%   default to one and zero respectively.
%   setStop(...,POS) Specify the position of the stop in the lane.
%   setStop(...,LANEINDEX) Specify the lane index in which the stop will be
%   made.
%   setStop(...,DURATION) Specify the duration of the stop.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Nicolás Arroyo, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setBusStop.m 51 2018-12-30 22:32:29Z afacostag $

import traci.constants

p = inputParser;
p.FunctionName = 'vehicle.setBusStop';
p.addRequired('vehID',@ischar)
p.addRequired('stopID',@ischar)
p.addOptional('duration', 2^31-1, @isnumeric)
p.addOptional('flags', constants.STOP_BUS_STOP, @isnumeric)
p.addOptional('until', -1, @isnumeric)
p.parse(vehID, stopID, varargin{:})

vehID = p.Results.vehID;
stopID = p.Results.stopID;
duration= p.Results.duration;
flags = p.Results.flags;
until = p.Results.until;

traci.vehicle.setStop(vehID,stopID, 'duration', duration,'until', until,...
    'flags', constants.STOP_BUS_STOP);
