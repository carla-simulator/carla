function setLinkState(tlsID, tlsLinkIndex, state)
%setLinkState Set the state for the given tls and link index.
%   setLinkState(TLSID,TLSLINKINDEX,STATE) Sets the state for the given tls
%   and link index. The state must be one of rRgGyYoOu for red, red-yellow,
%   green, yellow, off, where lower case letters mean that the stream has
%   to decelerate.
%   The link index is shown the gui when setting the appropriate junction
%   visualization option.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setLinkState.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

fullState = traci.trafficlights.getRedYellowGreenState(tlsID);
if tlsLinkIndex >= length(fullState)
    traci.close();
    throw(MException('TraCI4Matlab:InvalidArgument', sprintf('Invalid tlsLinkIndex %s for tls ''%s'' with maximum index %s.\n',...
        num2str(tlsLinkIndex), num2str(tlsID), num2str(length(fullState) - 1))));
else
    fullState(tlsLinkIndex + 1) = state;
    traci.trafficlights.setRedYellowGreenState(tlsID, fullState);
end
