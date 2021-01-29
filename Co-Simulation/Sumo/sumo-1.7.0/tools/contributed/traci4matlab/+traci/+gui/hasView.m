function b = hasView(varargin)
%hasView Check whether the given view exists.
%   b = hasView(VIEWID) Check whether the given view exists in the GUI.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: hasView.m 54 2019-01-03 15:41:54Z afacostag $

import traci.constants

if nargin < 1
    viewID = 'View #0';
else
    viewID = varargin{:};
end

b = traci.gui.getUniversal(constants.VAR_HAS_VIEW, viewID);
