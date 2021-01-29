--
-- Copyright (C) 2013 Christoph Sommer <christoph.sommer@uibk.ac.at>
--
-- Documentation for these modules is at http://veins.car2x.org/
--
-- SPDX-License-Identifier: GPL-2.0-or-later
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--

-- TraCI Dissector for Wireshark
-- Include from (or copy to) ~/.wireshark/init.lua

function traci_proto_commandIdToString(id)
	if id == 0x00 then return "CMD_GETVERSION" end
	if id == 0x02 then return "CMD_SIMSTEP2" end
	if id == 0x12 then return "CMD_STOP" end
	if id == 0x13 then return "CMD_CHANGELANE" end
	if id == 0x14 then return "CMD_SLOWDOWN" end
	if id == 0x31 then return "CMD_CHANGETARGET" end
	if id == 0x74 then return "CMD_ADDVEHICLE" end
	if id == 0x7F then return "CMD_CLOSE" end
	if id == 0x80 then return "CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT" end
	if id == 0xa0 then return "CMD_GET_INDUCTIONLOOP_VARIABLE" end
	if id == 0xd0 then return "CMD_SUBSCRIBE_INDUCTIONLOOP_VARIABLE" end
	if id == 0x81 then return "CMD_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_CONTEXT" end
	if id == 0xa1 then return "CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE" end
	if id == 0xd1 then return "CMD_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE" end
	if id == 0x82 then return "CMD_SUBSCRIBE_TL_CONTEXT" end
	if id == 0xa2 then return "CMD_GET_TL_VARIABLE" end
	if id == 0xc2 then return "CMD_SET_TL_VARIABLE" end
	if id == 0xd2 then return "CMD_SUBSCRIBE_TL_VARIABLE" end
	if id == 0x83 then return "CMD_SUBSCRIBE_LANE_CONTEXT" end
	if id == 0xa3 then return "CMD_GET_LANE_VARIABLE" end
	if id == 0xc3 then return "CMD_SET_LANE_VARIABLE" end
	if id == 0xd3 then return "CMD_SUBSCRIBE_LANE_VARIABLE" end
	if id == 0x84 then return "CMD_SUBSCRIBE_VEHICLE_CONTEXT" end
	if id == 0xa4 then return "CMD_GET_VEHICLE_VARIABLE" end
	if id == 0xc4 then return "CMD_SET_VEHICLE_VARIABLE" end
	if id == 0xd4 then return "CMD_SUBSCRIBE_VEHICLE_VARIABLE" end
	if id == 0x85 then return "CMD_SUBSCRIBE_VEHICLETYPE_CONTEXT" end
	if id == 0xa5 then return "CMD_GET_VEHICLETYPE_VARIABLE" end
	if id == 0xc5 then return "CMD_SET_VEHICLETYPE_VARIABLE" end
	if id == 0xd5 then return "CMD_SUBSCRIBE_VEHICLETYPE_VARIABLE" end
	if id == 0x86 then return "CMD_SUBSCRIBE_ROUTE_CONTEXT" end
	if id == 0xa6 then return "CMD_GET_ROUTE_VARIABLE" end
	if id == 0xc6 then return "CMD_SET_ROUTE_VARIABLE" end
	if id == 0xd6 then return "CMD_SUBSCRIBE_ROUTE_VARIABLE" end
	if id == 0x87 then return "CMD_SUBSCRIBE_POI_CONTEXT" end
	if id == 0xa7 then return "CMD_GET_POI_VARIABLE" end
	if id == 0xc7 then return "CMD_SET_POI_VARIABLE" end
	if id == 0xd7 then return "CMD_SUBSCRIBE_POI_VARIABLE" end
	if id == 0x88 then return "CMD_SUBSCRIBE_POLYGON_CONTEXT" end
	if id == 0xa8 then return "CMD_GET_POLYGON_VARIABLE" end
	if id == 0xc8 then return "CMD_SET_POLYGON_VARIABLE" end
	if id == 0xd8 then return "CMD_SUBSCRIBE_POLYGON_VARIABLE" end
	if id == 0x89 then return "CMD_SUBSCRIBE_JUNCTION_CONTEXT" end
	if id == 0xa9 then return "CMD_GET_JUNCTION_VARIABLE" end
	if id == 0xc9 then return "CMD_SET_JUNCTION_VARIABLE" end
	if id == 0xd9 then return "CMD_SUBSCRIBE_JUNCTION_VARIABLE" end
	if id == 0x8a then return "CMD_SUBSCRIBE_EDGE_CONTEXT" end
	if id == 0xaa then return "CMD_GET_EDGE_VARIABLE" end
	if id == 0xca then return "CMD_SET_EDGE_VARIABLE" end
	if id == 0xda then return "CMD_SUBSCRIBE_EDGE_VARIABLE" end
	if id == 0x8b then return "CMD_SUBSCRIBE_SIM_CONTEXT" end
	if id == 0xab then return "CMD_GET_SIM_VARIABLE" end
	if id == 0xcb then return "CMD_SET_SIM_VARIABLE" end
	if id == 0xdb then return "CMD_SUBSCRIBE_SIM_VARIABLE" end
	if id == 0x8c then return "CMD_SUBSCRIBE_GUI_CONTEXT" end
	if id == 0xac then return "CMD_GET_GUI_VARIABLE" end
	if id == 0xcc then return "CMD_SET_GUI_VARIABLE" end
	if id == 0xdc then return "CMD_SUBSCRIBE_GUI_VARIABLE" end
	if id == 0x90 then return "CMD_REROUTE_TRAVELTIME" end
	if id == 0x91 then return "CMD_REROUTE_EFFORT" end
	if id == 0x90 then return "RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT" end
	if id == 0xb0 then return "RESPONSE_GET_INDUCTIONLOOP_VARIABLE" end
	if id == 0xe0 then return "RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE" end
	if id == 0x91 then return "RESPONSE_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_CONTEXT" end
	if id == 0xb1 then return "RESPONSE_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE" end
	if id == 0xe1 then return "RESPONSE_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE" end
	if id == 0x92 then return "RESPONSE_SUBSCRIBE_TL_CONTEXT" end
	if id == 0xb2 then return "RESPONSE_GET_TL_VARIABLE" end
	if id == 0xe2 then return "RESPONSE_SUBSCRIBE_TL_VARIABLE" end
	if id == 0x93 then return "RESPONSE_SUBSCRIBE_LANE_CONTEXT" end
	if id == 0xb3 then return "RESPONSE_GET_LANE_VARIABLE" end
	if id == 0xe3 then return "RESPONSE_SUBSCRIBE_LANE_VARIABLE" end
	if id == 0x94 then return "RESPONSE_SUBSCRIBE_VEHICLE_CONTEXT" end
	if id == 0xb4 then return "RESPONSE_GET_VEHICLE_VARIABLE" end
	if id == 0xe4 then return "RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE" end
	if id == 0x95 then return "RESPONSE_SUBSCRIBE_VEHICLETYPE_CONTEXT" end
	if id == 0xb5 then return "RESPONSE_GET_VEHICLETYPE_VARIABLE" end
	if id == 0xe5 then return "RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE" end
	if id == 0x96 then return "RESPONSE_SUBSCRIBE_ROUTE_CONTEXT" end
	if id == 0xb6 then return "RESPONSE_GET_ROUTE_VARIABLE" end
	if id == 0xe6 then return "RESPONSE_SUBSCRIBE_ROUTE_VARIABLE" end
	if id == 0x97 then return "RESPONSE_SUBSCRIBE_POI_CONTEXT" end
	if id == 0xb7 then return "RESPONSE_GET_POI_VARIABLE" end
	if id == 0xe7 then return "RESPONSE_SUBSCRIBE_POI_VARIABLE" end
	if id == 0x98 then return "RESPONSE_SUBSCRIBE_POLYGON_CONTEXT" end
	if id == 0xb8 then return "RESPONSE_GET_POLYGON_VARIABLE" end
	if id == 0xe8 then return "RESPONSE_SUBSCRIBE_POLYGON_VARIABLE" end
	if id == 0x99 then return "RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT" end
	if id == 0xb9 then return "RESPONSE_GET_JUNCTION_VARIABLE" end
	if id == 0xe9 then return "RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE" end
	if id == 0x9a then return "RESPONSE_SUBSCRIBE_EDGE_CONTEXT" end
	if id == 0xba then return "RESPONSE_GET_EDGE_VARIABLE" end
	if id == 0xea then return "RESPONSE_SUBSCRIBE_EDGE_VARIABLE" end
	if id == 0x9b then return "RESPONSE_SUBSCRIBE_SIM_CONTEXT" end
	if id == 0xbb then return "RESPONSE_GET_SIM_VARIABLE" end
	if id == 0xeb then return "RESPONSE_SUBSCRIBE_SIM_VARIABLE" end
	if id == 0x9c then return "RESPONSE_SUBSCRIBE_GUI_CONTEXT" end
	if id == 0xbc then return "RESPONSE_GET_GUI_VARIABLE" end
	if id == 0xec then return "RESPONSE_SUBSCRIBE_GUI_VARIABLE" end
	return "unknown"
end

traci_proto = Proto("TraCI","TraCI (Traffic Control Interface)")
function traci_proto.dissector(buffer, pinfo, tree)

	pinfo.cols.protocol = "TraCI"

	local msgOffset = 0
	while (msgOffset < buffer:len()) do

		local msg = buffer(msgOffset)

		if (msg:len() < 4) then
			-- buffer contains less than 4 Bytes (which we need for reading the message length) -> request to be called again with more packets concatenated to the buffer
			pinfo.desegment_offset = msgOffset
			pinfo.desegment_len = 4 - msg:len()
			return nil
		end

		local messageLen = msg(0,4):uint()

		if (msg:len() < messageLen) then
			-- buffer contains less Bytes than message length -> request to be called again with more packets concatenated to the buffer
			pinfo.desegment_offset = msgOffset
			pinfo.desegment_len = messageLen - msg:len()
			return nil
		end

		--
		-- buffer contains (at least) one message -> start parsing message
		--

		local subtreeMsg = tree:add(traci_proto, msg(0, messageLen), "TraCI Message of length " .. messageLen)
		subtreeMsg:add(msg(0, 4), "Message length: " .. messageLen)

		cmdOffset = 4
		while (cmdOffset < messageLen) do
			-- read command length
			local commandLen = tonumber(msg(cmdOffset + 0, 1):uint())
			local commandLenExt = 0
			local cmdStartOffset = 1
			if commandLen == 0 then
				cmdStartOffset = 5
				commandLenExt = tonumber(msg(cmdOffset + 1, 4):uint())
			end
			local commandId = tonumber(msg(cmdOffset + cmdStartOffset, 1):uint())

			local subtreeCmd = subtreeMsg:add(traci_proto, msg(cmdOffset + 0, commandLen + commandLenExt), "Command 0x" .. string.format("%X", commandId) .. " (" .. traci_proto_commandIdToString(commandId) .. ")")
			subtreeCmd:add(msg(cmdOffset + 0, 1), "Command length: " .. commandLen)
			if commandLenExt > 0 then
				subtreeCmd:add(msg(cmdOffset + 1, 4), "Command length ext: " .. commandLenExt)
			end
			subtreeCmd:add(msg(cmdOffset + cmdStartOffset, 1), "Command id: 0x" .. string.format("%X", commandId) .. " (" .. traci_proto_commandIdToString(commandId) .. ")")
			if commandLen + commandLenExt - cmdStartOffset - 1 > 0 then
				subtreeCmd:add(msg(cmdOffset + cmdStartOffset + 1, commandLen + commandLenExt - cmdStartOffset - 1), "Data of length " .. commandLen + commandLenExt - cmdStartOffset - 1)
			end

			-- a CMD_SIMSTEP2 returned from the server gets special treatment: it is immediately followed by an (unframed) count of returned subscription results
			local wasSimstep = (commandId == 2) and (commandLen == 7) and (tonumber(msg(cmdOffset + cmdStartOffset + 1, 1):uint()) == 0)
			if wasSimstep then
				local numSubscriptions = tonumber(msg(cmdOffset + commandLen + commandLenExt, 4):uint())
				subtreeMsg:add(msg(cmdOffset + commandLen + commandLenExt, 4), "Number of subscription results: " .. numSubscriptions)
				cmdOffset = cmdOffset + 4
			end

			-- end of command, retry with rest of bytes in this message
			cmdOffset = cmdOffset + commandLen + commandLenExt
		end


		--- end of message, retry with rest of bytes in this packet
		msgOffset = msgOffset + messageLen
	end

end

tcp = DissectorTable.get("tcp.port")
tcp:add(9999,traci_proto)

-- end
