/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIImporter_ArcView.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Thimor Bohn
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Importer for networks stored in ArcView-shape format
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeomHelper.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBHelpers.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBTypeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netimport/NINavTeqHelper.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/common/FileHelpers.h>
#include "NILoader.h"
#include "NIImporter_ArcView.h"

#ifdef HAVE_GDAL
#if __GNUC__ > 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <ogrsf_frmts.h>
#if __GNUC__ > 3
#pragma GCC diagnostic pop
#endif
#endif


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods (interface in this case)
// ---------------------------------------------------------------------------
void
NIImporter_ArcView::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    if (!oc.isSet("shapefile-prefix")) {
        return;
    }
    // check whether the correct set of entries is given
    //  and compute both file names
    std::string dbf_file = oc.getString("shapefile-prefix") + ".dbf";
    std::string shp_file = oc.getString("shapefile-prefix") + ".shp";
    std::string shx_file = oc.getString("shapefile-prefix") + ".shx";
    // check whether the files do exist
    if (!FileHelpers::isReadable(dbf_file)) {
        WRITE_ERROR("File not accessible: " + dbf_file);
    }
    if (!FileHelpers::isReadable(shp_file)) {
        WRITE_ERROR("File not accessible: " + shp_file);
    }
    if (!FileHelpers::isReadable(shx_file)) {
        WRITE_ERROR("File not accessible: " + shx_file);
    }
    if (MsgHandler::getErrorInstance()->wasInformed()) {
        return;
    }
    // load the arcview files
    NIImporter_ArcView loader(oc,
                              nb.getNodeCont(), nb.getEdgeCont(), nb.getTypeCont(),
                              dbf_file, shp_file, oc.getBool("speed-in-kmh"));
    loader.load();
}



// ---------------------------------------------------------------------------
// loader methods
// ---------------------------------------------------------------------------
NIImporter_ArcView::NIImporter_ArcView(const OptionsCont& oc,
                                       NBNodeCont& nc,
                                       NBEdgeCont& ec,
                                       NBTypeCont& tc,
                                       const std::string& dbf_name,
                                       const std::string& shp_name,
                                       bool speedInKMH)
    : myOptions(oc), mySHPName(shp_name),
      myNameAddition(0),
      myNodeCont(nc), myEdgeCont(ec), myTypeCont(tc),
      mySpeedInKMH(speedInKMH),
      myRunningEdgeID(0),
      myRunningNodeID(0) {
    UNUSED_PARAMETER(dbf_name);
}


NIImporter_ArcView::~NIImporter_ArcView() {}


void
NIImporter_ArcView::load() {
#ifdef HAVE_GDAL
    PROGRESS_BEGIN_MESSAGE("Loading data from '" + mySHPName + "'");
#if GDAL_VERSION_MAJOR < 2
    OGRRegisterAll();
    OGRDataSource* poDS = OGRSFDriverRegistrar::Open(mySHPName.c_str(), FALSE);
#else
    GDALAllRegister();
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mySHPName.c_str(), GDAL_OF_VECTOR | GA_ReadOnly, NULL, NULL, NULL);
#endif
    if (poDS == NULL) {
        WRITE_ERROR("Could not open shape description '" + mySHPName + "'.");
        return;
    }

    // begin file parsing
    OGRLayer*  poLayer = poDS->GetLayer(0);
    poLayer->ResetReading();

    // build coordinate transformation
    OGRSpatialReference* origTransf = poLayer->GetSpatialRef();
    OGRSpatialReference destTransf;
    // use wgs84 as destination
    destTransf.SetWellKnownGeogCS("WGS84");
#if GDAL_VERSION_MAJOR > 2
    if (myOptions.getBool("shapefile.traditional-axis-mapping")) {
        destTransf.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    }
#endif
    OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(origTransf, &destTransf);
    if (poCT == nullptr) {
        if (myOptions.isSet("shapefile.guess-projection")) {
            OGRSpatialReference origTransf2;
            origTransf2.SetWellKnownGeogCS("WGS84");
            poCT = OGRCreateCoordinateTransformation(&origTransf2, &destTransf);
        }
        if (poCT == nullptr) {
            WRITE_WARNING("Could not create geocoordinates converter; check whether proj.4 is installed.");
        }
    }

    const bool saveOrigIDs = OptionsCont::getOptions().getBool("output.original-names");
    OGRFeature* poFeature;
    poLayer->ResetReading();

    const double nodeJoinDist = myOptions.getFloat("shapefile.node-join-dist");
    const std::vector<std::string> params = myOptions.getStringVector("shapefile.add-params");

    int featureIndex = 0;
    bool warnNotUnique = true;
    std::string idPrefix = ""; // prefix for non-unique street-id values
    std::map<std::string, int> idIndex; // running index to make street-id unique
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        // read in edge attributes
        if (featureIndex == 0) {
            WRITE_MESSAGE("Available fields: " + toString(getFieldNames(poFeature)));
        }
        std::string id;
        std::string name;
        std::string from_node;
        std::string to_node;
        if (!getStringEntry(poFeature, "shapefile.street-id", "LINK_ID", true, id)) {
            WRITE_ERROR("Needed field '" + id + "' (street-id) is missing.");
            id = "";
        }
        if (id == "") {
            id = toString(myRunningEdgeID++);
        }

        getStringEntry(poFeature, "shapefile.name", "ST_NAME", true, name);
        name = StringUtils::replace(name, "&", "&amp;");

        if (!getStringEntry(poFeature, "shapefile.from-id", "REF_IN_ID", true, from_node)) {
            WRITE_ERROR("Needed field '" + from_node + "' (from node id) is missing.");
            from_node = "";
        }
        if (!getStringEntry(poFeature, "shapefile.to-id", "NREF_IN_ID", true, to_node)) {
            WRITE_ERROR("Needed field '" + to_node + "' (to node id) is missing.");
            to_node = "";
        }

        if (from_node == "" || to_node == "") {
            from_node = toString(myRunningNodeID++);
            to_node = toString(myRunningNodeID++);
        }

        std::string type;
        if (myOptions.isSet("shapefile.type-id") && poFeature->GetFieldIndex(myOptions.getString("shapefile.type-id").c_str()) >= 0) {
            type = poFeature->GetFieldAsString(myOptions.getString("shapefile.type-id").c_str());
        } else if (poFeature->GetFieldIndex("ST_TYP_AFT") >= 0) {
            type = poFeature->GetFieldAsString("ST_TYP_AFT");
        }
        if ((type != "" || myOptions.isSet("shapefile.type-id")) && !myTypeCont.knows(type)) {
            WRITE_WARNINGF("Unknown type '%' for edge '%'", type, id);
        }
        double width = myTypeCont.getWidth(type);
        bool oneway = myTypeCont.knows(type) ? myTypeCont.getIsOneWay(type) : false;
        double speed = getSpeed(*poFeature, id);
        int nolanes = getLaneNo(*poFeature, id, speed);
        int priority = getPriority(*poFeature, id);
        if (nolanes <= 0 || speed <= 0) {
            if (myOptions.getBool("shapefile.use-defaults-on-failure")) {
                nolanes = nolanes <= 0 ? myTypeCont.getNumLanes(type) : nolanes;
                speed = speed <= 0 ? myTypeCont.getSpeed(type) : speed;
            } else {
                const std::string lanesField = myOptions.isSet("shapefile.laneNumber") ? myOptions.getString("shapefile.laneNumber") : "nolanes";
                const std::string speedField = myOptions.isSet("shapefile.speed") ? myOptions.getString("shapefile.speed") : "speed";
                WRITE_ERROR("Required field '" + lanesField + "' or '" + speedField + "' is missing (add fields or set option --shapefile.use-defaults-on-failure).");
                WRITE_ERROR("Available fields: " + toString(getFieldNames(poFeature)));
                OGRFeature::DestroyFeature(poFeature);
                return;
            }
        }
        if (mySpeedInKMH) {
            speed = speed / (double) 3.6;
        }


        // read in the geometry
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        OGRwkbGeometryType gtype = poGeometry->getGeometryType();
        if (gtype != wkbLineString && gtype != wkbLineString25D) {
            OGRFeature::DestroyFeature(poFeature);
            WRITE_ERROR("Road geometry must be of type 'linestring' or 'linestring25D' (found '" + toString(gtype) + "')");
            return;
        }
        OGRLineString* cgeom = (OGRLineString*) poGeometry;
        if (poCT != nullptr) {
            // try transform to wgs84
            cgeom->transform(poCT);
        }

        PositionVector shape;
        for (int j = 0; j < cgeom->getNumPoints(); j++) {
            Position pos((double) cgeom->getX(j), (double) cgeom->getY(j), (double) cgeom->getZ(j));
            if (!NBNetBuilder::transformCoordinate(pos)) {
                WRITE_WARNINGF("Unable to project coordinates for edge '%'.", id);
            }
            shape.push_back_noDoublePos(pos);
        }

        // build from-node
        NBNode* from = myNodeCont.retrieve(from_node);
        if (from == 0) {
            Position from_pos = shape[0];
            from = myNodeCont.retrieve(from_pos, nodeJoinDist);
            if (from == 0) {
                from = new NBNode(from_node, from_pos);
                if (!myNodeCont.insert(from)) {
                    WRITE_ERROR("Node '" + from_node + "' could not be added");
                    delete from;
                    continue;
                }
            }
        }
        // build to-node
        NBNode* to = myNodeCont.retrieve(to_node);
        if (to == 0) {
            Position to_pos = shape[-1];
            to = myNodeCont.retrieve(to_pos, nodeJoinDist);
            if (to == 0) {
                to = new NBNode(to_node, to_pos);
                if (!myNodeCont.insert(to)) {
                    WRITE_ERROR("Node '" + to_node + "' could not be added");
                    delete to;
                    continue;
                }
            }
        }

        if (from == to) {
            WRITE_WARNINGF("Edge '%' connects identical nodes, skipping.", id);
            continue;
        }

        // retrieve the information whether the street is bi-directional
        std::string dir;
        int index = poFeature->GetDefnRef()->GetFieldIndex("DIR_TRAVEL");
        if (index >= 0 && poFeature->IsFieldSet(index)) {
            dir = poFeature->GetFieldAsString(index);
        }
        const std::string origID = saveOrigIDs ? id : "";
        // check for duplicate ids
        NBEdge* existing = myEdgeCont.retrieve(id);
        NBEdge* existingReverse = myEdgeCont.retrieve("-" + id);
        if (existing != nullptr || existingReverse != nullptr) {
            std::string duplicateID = existing != nullptr ? id : existingReverse->getID();
            if ((existing != 0 && existing->getGeometry() == shape)
                    || (existingReverse != 0 && existingReverse->getGeometry() == shape.reverse())) {
                WRITE_ERROR("Edge '" + duplicateID + " is not unique");
            } else {
                if (idIndex.count(id) == 0) {
                    idIndex[id] = 0;
                }
                idIndex[id]++;
                idPrefix = id;
                id += "#" + toString(idIndex[id]);
                if (warnNotUnique) {
                    WRITE_WARNING("street-id '" + idPrefix + "' is not unique. Renaming subsequent edge to '" + id + "'");
                    warnNotUnique = false;
                }
            }
        }
        // add positive direction if wanted
        if (dir == "B" || dir == "F" || dir == "" || myOptions.getBool("shapefile.all-bidirectional")) {
            if (myEdgeCont.retrieve(id) == 0) {
                LaneSpreadFunction spread = dir == "B" || dir == "FALSE" ? LaneSpreadFunction::RIGHT : LaneSpreadFunction::CENTER;
                NBEdge* edge = new NBEdge(id, from, to, type, speed, nolanes, priority, width, NBEdge::UNSPECIFIED_OFFSET, shape, name, origID, spread);
                edge->setPermissions(myTypeCont.getPermissions(type));
                myEdgeCont.insert(edge);
                checkSpread(edge);
                addParams(edge, poFeature, params);
            } else {
                WRITE_ERROR("Could not create edge '" + id + "'. An edge with the same id already exists");
            }
        }
        // add negative direction if wanted
        if ((dir == "B" || dir == "T" || myOptions.getBool("shapefile.all-bidirectional")) && !oneway) {
            if (myEdgeCont.retrieve("-" + id) == 0) {
                LaneSpreadFunction spread = dir == "B" || dir == "FALSE" ? LaneSpreadFunction::RIGHT : LaneSpreadFunction::CENTER;
                NBEdge* edge = new NBEdge("-" + id, to, from, type, speed, nolanes, priority, width, NBEdge::UNSPECIFIED_OFFSET, shape.reverse(), name, origID, spread);
                edge->setPermissions(myTypeCont.getPermissions(type));
                myEdgeCont.insert(edge);
                checkSpread(edge);
                addParams(edge, poFeature, params);
            } else {
                WRITE_ERROR("Could not create edge '-" + id + "'. An edge with the same id already exists");
            }
        }
        //
        OGRFeature::DestroyFeature(poFeature);
        featureIndex++;
    }
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource::DestroyDataSource(poDS);
#else
    GDALClose(poDS);
#endif
    PROGRESS_DONE_MESSAGE();
#else
    WRITE_ERROR("SUMO was compiled without GDAL support.");
#endif
}

#ifdef HAVE_GDAL
double
NIImporter_ArcView::getSpeed(OGRFeature& poFeature, const std::string& edgeid) {
    if (myOptions.isSet("shapefile.speed")) {
        int index = poFeature.GetDefnRef()->GetFieldIndex(myOptions.getString("shapefile.speed").c_str());
        if (index >= 0 && poFeature.IsFieldSet(index)) {
            const double speed = poFeature.GetFieldAsDouble(index);
            if (speed <= 0) {
                WRITE_WARNING("invalid value for field: '"
                              + myOptions.getString("shapefile.laneNumber")
                              + "': '" + std::string(poFeature.GetFieldAsString(index)) + "'");
            } else {
                return speed;
            }
        }
    }
    if (myOptions.isSet("shapefile.type-id")) {
        return myTypeCont.getSpeed(poFeature.GetFieldAsString((char*)(myOptions.getString("shapefile.type-id").c_str())));
    }
    // try to get definitions as to be found in SUMO-XML-definitions
    //  idea by John Michael Calandrino
    int index = poFeature.GetDefnRef()->GetFieldIndex("speed");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return (double) poFeature.GetFieldAsDouble(index);
    }
    index = poFeature.GetDefnRef()->GetFieldIndex("SPEED");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return (double) poFeature.GetFieldAsDouble(index);
    }
    // try to get the NavTech-information
    index = poFeature.GetDefnRef()->GetFieldIndex("SPEED_CAT");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        std::string def = poFeature.GetFieldAsString(index);
        return NINavTeqHelper::getSpeed(edgeid, def);
    }
    return -1;
}


int
NIImporter_ArcView::getLaneNo(OGRFeature& poFeature, const std::string& edgeid,
                              double speed) {
    if (myOptions.isSet("shapefile.laneNumber")) {
        int index = poFeature.GetDefnRef()->GetFieldIndex(myOptions.getString("shapefile.laneNumber").c_str());
        if (index >= 0 && poFeature.IsFieldSet(index)) {
            const int laneNumber = poFeature.GetFieldAsInteger(index);
            if (laneNumber <= 0) {
                WRITE_WARNING("invalid value for field '"
                              + myOptions.getString("shapefile.laneNumber")
                              + "': '" + std::string(poFeature.GetFieldAsString(index)) + "'");
            } else {
                return laneNumber;
            }
        }
    }
    if (myOptions.isSet("shapefile.type-id")) {
        return (int) myTypeCont.getNumLanes(poFeature.GetFieldAsString((char*)(myOptions.getString("shapefile.type-id").c_str())));
    }
    // try to get definitions as to be found in SUMO-XML-definitions
    //  idea by John Michael Calandrino
    int index = poFeature.GetDefnRef()->GetFieldIndex("nolanes");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return (int) poFeature.GetFieldAsInteger(index);
    }
    index = poFeature.GetDefnRef()->GetFieldIndex("NOLANES");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return (int) poFeature.GetFieldAsInteger(index);
    }
    index = poFeature.GetDefnRef()->GetFieldIndex("rnol");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return (int) poFeature.GetFieldAsInteger(index);
    }
    index = poFeature.GetDefnRef()->GetFieldIndex("LANE_CAT");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        std::string def = poFeature.GetFieldAsString(index);
        return NINavTeqHelper::getLaneNumber(edgeid, def, speed);
    }
    return 0;
}


int
NIImporter_ArcView::getPriority(OGRFeature& poFeature, const std::string& /*edgeid*/) {
    if (myOptions.isSet("shapefile.type-id")) {
        return myTypeCont.getPriority(poFeature.GetFieldAsString((char*)(myOptions.getString("shapefile.type-id").c_str())));
    }
    // try to get definitions as to be found in SUMO-XML-definitions
    //  idea by John Michael Calandrino
    int index = poFeature.GetDefnRef()->GetFieldIndex("priority");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return poFeature.GetFieldAsInteger(index);
    }
    index = poFeature.GetDefnRef()->GetFieldIndex("PRIORITY");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return poFeature.GetFieldAsInteger(index);
    }
    // try to determine priority from NavTechs FUNC_CLASS attribute
    index = poFeature.GetDefnRef()->GetFieldIndex("FUNC_CLASS");
    if (index >= 0 && poFeature.IsFieldSet(index)) {
        return poFeature.GetFieldAsInteger(index);
    }
    return 0;
}

void
NIImporter_ArcView::checkSpread(NBEdge* e) {
    NBEdge* ret = e->getToNode()->getConnectionTo(e->getFromNode());
    if (ret != 0) {
        e->setLaneSpreadFunction(LaneSpreadFunction::RIGHT);
        ret->setLaneSpreadFunction(LaneSpreadFunction::RIGHT);
    }
}

bool
NIImporter_ArcView::getStringEntry(OGRFeature* poFeature, const std::string& optionName, const char* defaultName, bool prune, std::string& into) {
    std::string v(defaultName);
    if (myOptions.isSet(optionName)) {
        v = myOptions.getString(optionName);
    }
    if (poFeature->GetFieldIndex(v.c_str()) < 0) {
        if (myOptions.isSet(optionName)) {
            into = v;
            return false;
        }
        into = "";
        return true;
    }
    into = poFeature->GetFieldAsString((char*)v.c_str());
    if (prune) {
        into = StringUtils::prune(into);
    }
    return true;
}

std::vector<std::string>
NIImporter_ArcView::getFieldNames(OGRFeature* poFeature) const {
    std::vector<std::string> fields;
    for (int i = 0; i < poFeature->GetFieldCount(); i++) {
        fields.push_back(poFeature->GetFieldDefnRef(i)->GetNameRef());
    }
    return fields;
}

void
NIImporter_ArcView::addParams(NBEdge* edge, OGRFeature* poFeature, const std::vector<std::string>& params) const {
    for (const std::string& p : params) {
        int index = poFeature->GetDefnRef()->GetFieldIndex(p.c_str());
        if (index >= 0 && poFeature->IsFieldSet(index)) {
            edge->setParameter(p, poFeature->GetFieldAsString(index));
        }
    }
}

#endif


/****************************************************************************/
