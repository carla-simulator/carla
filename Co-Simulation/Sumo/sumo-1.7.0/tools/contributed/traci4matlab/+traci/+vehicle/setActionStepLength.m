function setActionStepLength(vehID, actionStepLength, varargin)
%setActionStepLength Sets the action step length for this vehicle.
%   setActionStepLength(VEHID,ACTIONSTEPLENGTH) Sets the action step length
%   for this vehicle. The next action point is scheduled immediately.
%   setActionStepLength(...,RESETACTIONOFFSET) If resetActionOffset == True
%   (default), the next action point is scheduled immediately. if If
%   resetActionOffset == False, the interval between the last and the next
%   action point is updated to match the given value, or if the latter is
%   smaller than the time since the last action point, the next action
%   follows immediately.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setActionStepLength.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

if actionStepLength < 0
    traci.close()
    throw(MException('TraCI4Matlab:InvalidArgument', 'Invalid value for actionStepLength. Given value must be non-negative.'));
end

resetActionOffset = true;
if nargin > 2
    resetActionOffset = varargin{1};
end

if ~resetActionOffset
    actionStepLength = -actionStepLength;
end

traci.sendDoubleCmd(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.VAR_ACTIONSTEPLENGTH, vehID, actionStepLength);
