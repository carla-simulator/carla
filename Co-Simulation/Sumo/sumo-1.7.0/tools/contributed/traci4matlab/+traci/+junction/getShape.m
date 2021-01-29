function shape = getShape(junctionID)
%getShape List of 2D positions (cartesian) describing the geometry.
%   shape = getShape(JUNCTIONID) List of 2D positions (cartesian)
%   describing the geometry of the given junction.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: getShape.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

shape = traci.junction.getUniversal(constants.VAR_SHAPE, junctionID);
