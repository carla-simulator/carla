function couldIt = wantsAndCouldChangeLane(vehID, direction, varargin)
%wantsAndCouldChangeLane Return whether the vehicle wants to and could change lanes.
%   couldIt = wantsAndCouldChangeLane(VEHID,DIRECTION) Return whether the
%   vehicle wants to and could change lanes in the specified direction.
%   couldIt = wantsAndCouldChangeLane(...,STATE) Specify the lane change
%   state.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: wantsAndCouldChangeLane.m 54 2019-01-03 15:41:54Z afacostag $

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

if bitand(state, LCA_BLOCKED) == 0
    if direction == -1
        couldIt = bitand(state, constants.LCA_RIGHT) ~= 0;
        return
    end
    if direction == 1
        couldIt = bitand(state, constants.LCA_LEFT) ~= 0;
        return
    end
end
couldIt = false;
