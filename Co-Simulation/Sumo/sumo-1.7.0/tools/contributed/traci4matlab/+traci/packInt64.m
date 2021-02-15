function packedData = packInt64(data)
%packInt32 Internal function to cast an int64 into 
%	an uint8 array

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: packInt64.m 48 2018-12-26 15:35:20Z afacostag $

% if isa(data,'int64')
	packedData = fliplr(typecast(data,'uint8'));
% else
% 	packedData = fliplr(typecast(int64(data),'uint8'));
% end