function setPhase(tlsID, index)
%setPhase Set the phase index of the traffic light.
%   setPhase(TLSID,INDEX) Sets the index of the traffic lights with ID TLSID
%   to the given in the INDEX parameter. There are as many tls indexes as
%   phase definitions in the tls program. The tls index starts from zero.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setPhase.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendIntCmd(constants.CMD_SET_TL_VARIABLE,...
    constants.TL_PHASE_INDEX, tlsID, index);