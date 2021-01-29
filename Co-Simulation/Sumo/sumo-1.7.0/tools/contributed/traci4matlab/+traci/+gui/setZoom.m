function setZoom(viewID, zoom)
%setZoom Set the zoom of the view.
%   setZoom(VIEWID, ZOOM) Set the current zoom factor for the given view.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: setZoom.m 48 2018-12-26 15:35:20Z afacostag $

import traci.constants
traci.sendDoubleCmd(constants.CMD_SET_GUI_VARIABLE, constants.VAR_VIEW_ZOOM, viewID, zoom);