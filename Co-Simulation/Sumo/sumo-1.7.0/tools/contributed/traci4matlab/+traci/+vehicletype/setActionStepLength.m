function setActionStepLength(typeID, actionStepLength, varargin)
%setActionStepLength Sets the action step length for vehicles of this type.
%   setActionStepLength(TYPEID,ACTIONSTEPLENGTH) If resetActionOffset == True
%   (default), the next action point is scheduled immediately for all
%   vehicles of the type.
%   setActionStepLength(...,RESETACTIONOFFSET) If resetActionOffset == False,...
%   the interval between the last and the next action point is updated to
%   match the given value for all vehicles of the type, or if the latter is
%   smaller than the time since the last action point, the next action
%   follows immediately.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setActionStepLength.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

resetActionOffset = true;

if nargin > 2
    resetActionOffset = varargin{1};
end

if actionStepLength < 0
    traci.close();
    throw(MException('TraCI4Matlab:InvalidArgument',...
        'Invalid value for actionStepLength. Given value must be non-negative.'));
end

if ~resetActionOffset
    actionStepLength = -actionStepLength;
end

traci.sendDoubleCmd(constants.CMD_SET_VEHICLETYPE_VARIABLE,...
    constants.VAR_ACTIONSTEPLENGTH, typeID, actionStepLength);
