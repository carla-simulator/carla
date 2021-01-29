function setStop(vehID, edgeID, varargin)
%setStop Set a stop for the vehicle.
%   setStop(VEHID,EDGEID) Sets a stop for the vehicle VEHID in the edge
%   EDGEID for the maximum allowed time. The position and the lane index
%   default to one and zero respectively.
%   setStop(...,POS) Specify the position of the stop in the lane.
%   setStop(...,LANEINDEX) Specify the lane index in which the stop will be
%   made.
%   setStop(...,DURATION) Specify the duration of the stop.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Christian Portilla, Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setStop.m 51 2018-12-30 22:32:29Z afacostag $


import traci.constants
global message

p = inputParser;
p.FunctionName = 'vehicle.setStop';
p.addRequired('vehID',@ischar)
p.addRequired('edgeID',@ischar)
p.addOptional('pos', 1, @isnumeric)
p.addOptional('laneIndex', 0, @isnumeric)
p.addOptional('duration', constants.INVALID_DOUBLE_VALUE, @isnumeric)
p.addOptional('flags', constants.STOP_DEFAULT, @ischar)
p.addOptional('startPos', constants.INVALID_DOUBLE_VALUE, @isnumeric)
p.addOptional('until', constants.INVALID_DOUBLE_VALUE, @isnumeric)
p.parse(vehID, edgeID, varargin{:})

vehID = p.Results.vehID;
edgeID = p.Results.edgeID;
pos = p.Results.pos;
laneIndex = p.Results.laneIndex;
duration= p.Results.duration;
flags = p.Results.flags;
startPos = p.Results.startPos;
until = p.Results.until;

if duration >= 1000 && rem(duration,1000) == 0
  warning('API change now handles duration as floating point seconds');
end

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_STOP,...
    vehID, 1+4+1+4+length(edgeID)+1+8+1+1+1+8+1+1+1+8+1+8);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(7)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING,'%x')) ...
    traci.packInt32(length(edgeID)) uint8(edgeID)];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(pos) uint8([sscanf(constants.TYPE_BYTE,'%x') ...
    laneIndex sscanf(constants.TYPE_DOUBLE,'%x')]) ...
    traci.packInt64(duration) uint8([sscanf(constants.TYPE_BYTE,'%x') sscanf(flags, '%x')])];
message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(startPos) sscanf(constants.TYPE_DOUBLE,'%x') traci.packInt64(until)];
traci.sendExact();
