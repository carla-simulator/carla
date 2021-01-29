function setSpeed(personID, speed)
%setSpeed Set maximum speed.
%   setSpeed(PERSONID) Sets the maximum speed in m/s for the named person
%   for subsequent step.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setSpeed.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

traci.sendDoubleCmd(constants.CMD_SET_PERSON_VARIABLE, constants.VAR_SPEED,...
    personID, speed);
