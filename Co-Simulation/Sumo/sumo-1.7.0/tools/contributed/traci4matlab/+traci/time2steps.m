function timeInMiliseconds = time2steps(time)
%timeInMiliseconds An internal function to convert time in seconds to
%miliseconds.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: time2steps.m 48 2018-12-26 15:35:20Z afacostag $

timeInMiliseconds = int32(time*1000);