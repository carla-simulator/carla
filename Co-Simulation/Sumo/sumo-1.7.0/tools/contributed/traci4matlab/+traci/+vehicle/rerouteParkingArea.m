function rerouteParkingArea(vehID, parkingAreaID)
%rerouteParkingArea Changes the next parking area in parkingAreaID.
%   rerouteParkingArea(VEHID,PARKINGAREAID) Changes the next parking area in
%   parkingAreaID, updates the vehicle route, and preserve consistency in
%   case of passengers/containers on board.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: rerouteParkingArea.m 54 2019-01-03 15:41:54Z afacostag $

global message
import traci.constants

traci.beginMessage(constants.CMD_SET_VEHICLE_VARIABLE,...
    constants.CMD_REROUTE_TO_PARKING, vehID, 1+4+1+4+length(parkingAreaID));
message.string = [message.string uint8(sscanf(constants.TYPE_COMPOUND, '%x')) ...
    traci.packInt32(1)];
message.string = [message.string uint8(sscanf(constants.TYPE_STRING, '%x')) ...
    traci.packInt32(length(parkingAreaID)) uint8(parkingAreaID)];
traci.sendExact();
