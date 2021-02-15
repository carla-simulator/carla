function setProgram(tlsID, programID)
%setProgram Sets the id of the current program.
%   setProgram(TLSID,PROGRAMID)Sets the id of the current program.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setProgram.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendStringCmd(constants.CMD_SET_TL_VARIABLE,...
    constants.TL_PROGRAM, tlsID, programID);