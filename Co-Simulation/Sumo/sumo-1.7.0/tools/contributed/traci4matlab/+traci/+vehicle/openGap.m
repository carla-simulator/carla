function openGap(vehID, newTimeHeadway, newSpaceHeadway, duration,...
    changeRate, varargin)
%openGap Changes the vehicle's desired time headway.
%   openGap(VEHID,NEWTIMEHEADWAY,NEWSPACEHEADWAY,DURATION,CHANGERATE)
%   Changes the vehicle's desired time headway (cf-parameter tau) smoothly
%   to the given new value using the given change rate. Similarly, the
%   given space headway is applied gradually to achieve a minimal spatial
%   gap.
%   The vehicle is commanded to keep the increased headway for the given
%   duration once its target value is attained.
%   openGap(...,MAXDECEL) Optionally, a maximal value for the deceleration
%   (>0) can be given to prevent harsh braking due to the change of tau.
%   Note that this does only affect the following behavior regarding the
%   current leader and does not influence the gap acceptance during lane
%   change, etc.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: openGap.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants
global message

if duration > 1000
    warning('API change now handles duration as floating point seconds.')
end

maxDecel = -1;

if nargin > 5
    maxDecel = varargin{1};
end

nParams = 4 + int32(maxDecel > 0);
msgLength = 1+4+ (1+8)*nParams;

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE, constants.CMD_OPENGAP,...
    vehID, msgLength);
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND,'%x')) ...
    traci.packInt32(nParams) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(newTimeHeadway) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(newSpaceHeadway)  uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(duration) uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
    traci.packInt64(changeRate)];

if nParams == 5
    message.string = [message.string uint8(sscanf(constants.TYPE_DOUBLE,'%x')) ...
        traci.packInt64(maxDecel)];
end

traci.sendExact();
