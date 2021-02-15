/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    PCPolyContainer.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Melanie Knocke
/// @date    Mon, 05 Dec 2005
///
// A storage for loaded polygons and pois
/****************************************************************************/
#include <config.h>

#include <string>
#include <algorithm>
#include <map>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/shapes/SUMOPolygon.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include <utils/options/OptionsCont.h>
#include "PCPolyContainer.h"


// ===========================================================================
// method definitions
// ===========================================================================
PCPolyContainer::PCPolyContainer(bool prune,
                                 const Boundary& pruningBoundary,
                                 const std::vector<std::string>& removeByNames)
    : myPruningBoundary(pruningBoundary), myDoPrune(prune),
      myRemoveByNames(removeByNames) {}


PCPolyContainer::~PCPolyContainer() {
    myPolygons.clear();
    myPOIs.clear();
}


bool
PCPolyContainer::add(SUMOPolygon* poly, bool ignorePruning) {
    // check whether the polygon lies within the wished area
    //  - if such an area was given
    if (myDoPrune && !ignorePruning) {
        Boundary b = poly->getShape().getBoxBoundary();
        if (!b.partialWithin(myPruningBoundary)) {
            delete poly;
            return false;
        }
    }
    // check whether the polygon was named to be a removed one
    if (find(myRemoveByNames.begin(), myRemoveByNames.end(), poly->getID()) != myRemoveByNames.end()) {
        delete poly;
        return false;
    }
    return ShapeContainer::add(poly);
}


bool
PCPolyContainer::add(PointOfInterest* poi, bool ignorePruning) {
    // check whether the poi lies within the wished area
    //  - if such an area was given
    if (myDoPrune && !ignorePruning) {
        if (!myPruningBoundary.around(*poi)) {
            delete poi;
            return false;
        }
    }
    // check whether the polygon was named to be a removed one
    if (find(myRemoveByNames.begin(), myRemoveByNames.end(), poi->getID()) != myRemoveByNames.end()) {
        delete poi;
        return false;
    }
    return ShapeContainer::add(poi);
}


void
PCPolyContainer::addLanePos(const std::string& poiID, const std::string& laneID, double lanePos, double lanePosLat) {
    myLanePosPois[poiID] = LanePos(laneID, lanePos, lanePosLat);
}


void
PCPolyContainer::save(const std::string& file, bool useGeo) {
    const GeoConvHelper& gch = GeoConvHelper::getFinal();
    if (useGeo && !gch.usingGeoProjection()) {
        WRITE_WARNING("Ignoring option \"proj.plain-geo\" because no geo-conversion has been defined");
        useGeo = false;
    }
    OutputDevice& out = OutputDevice::getDevice(file);
    out.writeXMLHeader("additional", "additional_file.xsd");
    if (useGeo) {
        out.setPrecision(gPrecisionGeo);
    } else if (gch.usingGeoProjection()) {
        GeoConvHelper::writeLocation(out);
    }
    // write polygons
    for (auto i : myPolygons) {
        i.second->writeXML(out, useGeo);
    }
    // write pois
    const double zOffset = OptionsCont::getOptions().getFloat("poi-layer-offset");
    for (auto i : myPOIs) {
        std::map<std::string, LanePos>::const_iterator it = myLanePosPois.find(i.first);
        if (it == myLanePosPois.end()) {
            i.second->writeXML(out, useGeo, zOffset);
        } else {
            i.second->writeXML(out, useGeo, zOffset, it->second.laneID, it->second.pos, it->second.posLat);
        }
    }
    out.close();
}


void PCPolyContainer::writeDlrTDPHeader(OutputDevice& device, const OptionsCont& oc) {
    // XXX duplicate of NWWriter_DlrNavteq::writeHeader()
    device << "# Format matches Extraction version: V6.5 \n";
    std::stringstream tmp;
    oc.writeConfiguration(tmp, true, false, false);
    tmp.seekg(std::ios_base::beg);
    std::string line;
    while (!tmp.eof()) {
        std::getline(tmp, line);
        device << "# " << line << "\n";
    }
    device << "#\n";
}


void
PCPolyContainer::saveDlrTDP(const std::string& prefix) {
    const OptionsCont& oc = OptionsCont::getOptions();
    const GeoConvHelper& gch = GeoConvHelper::getFinal();
    const bool haveGeo = gch.usingGeoProjection();
    const double geoScale = pow(10.0f, haveGeo ? 5 : 2); // see NIImporter_DlrNavteq::GEO_SCALE
    // write pois
    OutputDevice& out = OutputDevice::getDevice(prefix + "_points_of_interest.txt");
    out.setPrecision(0);
    writeDlrTDPHeader(out, oc);
    // write format specifier
    out << "# ID\tCITY\tTYPE\tNAME\tgeo_x\tgeo_y\n";
    int id = 0;
    for (const auto& i : myPOIs) {
        Position pos(*i.second);
        gch.cartesian2geo(pos);
        pos.mul(geoScale);
        out << id << "\t";
        out << "" << "\t";
        out << i.second->getShapeType() << "\t";
        out << i.first << "\t";
        out << pos.x() << "\t";
        out << pos.y() << "\t";
        id++;
    }
    out.close();
    // write polygons
    OutputDevice& out2 = OutputDevice::getDevice(prefix + "_polygons.txt");
    out2.setPrecision(0);
    writeDlrTDPHeader(out2, oc);
    // write format specifier
    out2 << "# ID\tCITY\tTYPE\tNAME\tgeo_x1\tgeo_y1\t[geo_x2 geo_y2 ...]\n";
    id = 0;
    for (const auto& i : myPolygons) {
        out2 << id << "\t";
        out2 << "" << "\t";
        out2 << i.second->getShapeType() << "\t";
        out2 << i.first << "\t";

        PositionVector shape(i.second->getShape());
        for (int i = 0; i < (int) shape.size(); i++) {
            Position pos = shape[i];
            gch.cartesian2geo(pos);
            pos.mul(geoScale);
            out2 << pos.x() << "\t";
            out2 << pos.y() << "\t";
        }
        id++;
    }
    out2.close();
}


int
PCPolyContainer::getEnumIDFor(const std::string& key) {
    return myIDEnums[key]++;
}


/****************************************************************************/
