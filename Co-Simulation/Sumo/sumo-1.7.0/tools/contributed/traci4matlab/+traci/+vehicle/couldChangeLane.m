function couldIt = couldChangeLane(vehID, direction, varargin)
%couldChangeLane Return whether the vehicle could change lanes.
%   couldIt = couldChangeLane(VEHID,DIRECTION) Return whether the vehicle
%   could change lanes in the specified direction.
%   couldIt = couldChangeLane(...,STATE) Specify the lane change state.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: couldChangeLane.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

state = '';
if nargin > 2
    state = varargin{1};
end

if isempty(state)
    [state, stateTraCI] = traci.vehicle.getLaneChangeState(vehID, direction);
    if traci.vehicle.wantsAndCouldChangeLane(vehID, direction, stateTraCI)
        couldIt = false;
        return
    end
end

LCA_BLOCKED_LEFT = bitor(constants.LCA_BLOCKED_BY_LEFT_LEADER,constants.LCA_BLOCKED_BY_LEFT_FOLLOWER);
LCA_BLOCKED_RIGHT = bitor(constants.LCA_BLOCKED_BY_RIGHT_LEADER,constants.LCA_BLOCKED_BY_RIGHT_FOLLOWER);
LCA_BLOCKED = bitor(bitor(bitor(LCA_BLOCKED_LEFT,LCA_BLOCKED_RIGHT),...
        constants.LCA_INSUFFICIENT_SPACE),constants.LCA_INSUFFICIENT_SPEED);

couldIt = state ~= traci.constants.LCA_UNKNOWN && ...
    (bitand(state, LCA_BLOCKED) == 0);
