function setSignals(vehID, signals)
%setSignals Sets an integer encoding the state of the vehicle's signals.
%   setSignalsVEHID,SIGNALS) Sets an integer encoding the state of the 
%   vehicle's signals.
%   The following table shows the defined signals.
%
%   Name                            Bit
%   VEH_SIGNAL_BLINKER_RIGHT         0
%   VEH_SIGNAL_BLINKER_LEFT          1
%   VEH_SIGNAL_BLINKER_EMERGENCY	 2
%   VEH_SIGNAL_BRAKELIGHT            3
%   VEH_SIGNAL_FRONTLIGHT            4
%   VEH_SIGNAL_FOGLIGHT              5
%   VEH_SIGNAL_HIGHBEAM              6
%   VEH_SIGNAL_BACKDRIVE             7
%   VEH_SIGNAL_WIPER                 8
%   VEH_SIGNAL_DOOR_OPEN_LEFT        9
%   VEH_SIGNAL_DOOR_OPEN_RIGHT       10
%   VEH_SIGNAL_EMERGENCY_BLUE        11
%   VEH_SIGNAL_EMERGENCY_RED         12
%   VEH_SIGNAL_EMERGENCY_YELLOW      13

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setSignals.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendIntCmd(constants.CMD_SET_VEHICLE_VARIABLE, constants.VAR_SIGNALS, vehID, signals);