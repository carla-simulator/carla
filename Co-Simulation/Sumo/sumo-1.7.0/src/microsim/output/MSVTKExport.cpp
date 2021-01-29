/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    MSVTKExport.cpp
/// @author  Mario Krumnow
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    2012-04-26
///
// Realises VTK Export
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSJunctionControl.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include "MSVTKExport.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
MSVTKExport::write(OutputDevice& of, SUMOTime /* timestep */) {

    std::vector<double> speed = getSpeed();
    std::vector<double> points = getPositions();

    of << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    of << "<VTKFile type=\"PolyData\" version=\"0.1\" order=\"LittleEndian\">\n";
    of << "<PolyData>\n";
    of << " <Piece NumberOfPoints=\"" << speed.size() << "\" NumberOfVerts=\"1\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">\n";
    of << "<PointData>\n";
    of << " <DataArray type=\"Float64\" Name=\"speed\" format=\"ascii\">" << List2String(getSpeed()) << "</DataArray>\n";
    of << "</PointData>\n";
    of << "<CellData/>\n";
    of << "<Points>\n";
    of << " <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">" << List2String(getPositions()) << "</DataArray>\n";
    of << "</Points>\n";
    of << "<Verts>\n";
    of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">" <<  getOffset((int) speed.size()) << "</DataArray>\n";
    of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">" << speed.size() << "</DataArray>\n";
    of << "</Verts>\n";
    of << "<Lines>\n";
    of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\"/>\n";
    of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\"/>\n";
    of << "</Lines>\n";
    of << "<Stripes>\n";
    of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\"/>\n";
    of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\"/>\n";
    of << "</Stripes>\n";
    of << "<Polys>\n";
    of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\"/>\n";
    of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\"/>\n";
    of << "</Polys>\n";
    of << "</Piece>\n";
    of << "</PolyData>\n";
    of << "</VTKFile>";

}

std::vector<double>
MSVTKExport::getSpeed() {

    std::vector<double> output;

    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();


    for (; it != end; ++it) {
        const MSVehicle* veh = static_cast<const MSVehicle*>((*it).second);

        if (veh->isOnRoad()) {

            Position pos = veh->getLane()->getShape().positionAtOffset(veh->getPositionOnLane());
            output.push_back(veh->getSpeed());
        }

    }

    return output;
}

std::vector<double>
MSVTKExport::getPositions() {

    std::vector<double> output;

    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();


    for (; it != end; ++it) {
        const MSVehicle* veh = static_cast<const MSVehicle*>((*it).second);

        if (veh->isOnRoad()) {

            output.push_back(veh->getPosition().x());
            output.push_back(veh->getPosition().y());
            output.push_back(veh->getPosition().z());

        }

    }

    return output;
}

std::string
MSVTKExport::List2String(std::vector<double> input) {

    std::string output = "";
    for (int i = 0; i < (int)input.size(); i++) {

        std::stringstream ss;

        //for a high precision
        //ss.precision(::std::numeric_limits<double>::digits10);
        //ss.unsetf(::std::ios::dec);
        //ss.setf(::std::ios::scientific);

        ss << input[i] << " ";
        output += ss.str();
    }

    return trim(output);
}

std::string
MSVTKExport::getOffset(int nr) {

    std::string output = "";
    for (int i = 0; i < nr; i++) {

        std::stringstream ss;
        ss << i << " ";
        output += ss.str();
    }

    return trim(output);
}

bool
MSVTKExport::ctype_space(const char c) {
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 11) {
        return true;
    }
    return false;
}

std::string
MSVTKExport::trim(std::string istring) {
    bool trimmed = false;

    if (ctype_space(istring[istring.length() - 1])) {
        istring.erase(istring.length() - 1);
        trimmed = true;
    }

    if (ctype_space(istring[0])) {
        istring.erase(0, 1);
        trimmed = true;
    }

    if (!trimmed) {
        return istring;
    } else {
        return trim(istring);
    }

}


/****************************************************************************/
