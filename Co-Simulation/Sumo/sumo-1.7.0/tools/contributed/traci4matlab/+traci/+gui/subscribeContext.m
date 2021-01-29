function subscribeContext(viewID, domain, dist, varargin) 
%subscribeContext Subscribe to a view's context variable.
%   subscribeContext(VIEWID,DOMAIN,DIST) Subscribe to the VAR_VIEW_OFFSET 
%   value of the SUMO objects that surround the view specified by VIEWID at
%   a distance given by DIST, for the maximum allowed interval. The type of
%   objets that surround the view are defined in the DOMAIN parameter. Note
%   that not all the SUMO object types support the variable 
%   VAR_VIEW. 
%   subscribeContext(...,VARIDS) Subscribe to the values given in the cell 
%   array of strings VARIDS.
%   subscribeContext(...,BEGIN) Subscribe from the time BEGIN to the
%   maximum allowed end time.
%   subscribeContext(...,END) Subscribe for the time interval defined by 
%   BEGIN and END. 
%   A call to this method clears all previous subscription results.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: subscribeContext.m 48 2018-12-26 15:35:20Z afacostag $

global guiSubscriptionResults
import traci.constants

% Parse the input
p = inputParser;
p.FunctionName = 'gui.subscribeContext';
p.addRequired('viewID',@ischar)
p.addRequired('domain',@ischar)
p.addRequired('dist',@isnumeric)
p.addOptional('varIDs', {constants.VAR_VIEW_OFFSET}, @iscell)
p.addOptional('subscriptionBegin', 0, @(x)isnumeric(x) && length(x)==1)
p.addOptional('subscriptionEnd', 2^31-1, @(x)isnumeric(x) && length(x)==1)
p.parse(viewID, domain, dist, varargin{:})
viewID = p.Results.viewID;
domain = p.Results.domain;
dist = p.Results.dist;
varIDs = p.Results.varIDs;
subscriptionBegin = p.Results.subscriptionBegin;
subscriptionEnd = p.Results.subscriptionEnd;

guiSubscriptionResults = traci.SubscriptionResults(traci.RETURN_VALUE_FUNC.gui);

guiSubscriptionResults.reset()
traci.subscribeContext(constants.CMD_SUBSCRIBE_GUI_CONTEXT,...
    subscriptionBegin, subscriptionEnd, viewID, domain, dist, varIDs)