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
/// @file    NIXMLTypesHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    Tue, 20 Nov 2001
///
// Importer for edge type information stored in XML
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/SUMOVehicleClass.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBTypeCont.h>
#include "NIXMLTypesHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIXMLTypesHandler::NIXMLTypesHandler(NBTypeCont& tc)
    : SUMOSAXHandler("xml-types - file"),
      myTypeCont(tc) {}


NIXMLTypesHandler::~NIXMLTypesHandler() {}


void
NIXMLTypesHandler::myStartElement(int element,
                                  const SUMOSAXAttributes& attrs) {
    switch (element) {
        case SUMO_TAG_TYPE: {
            bool ok = true;
            // get the id, report a warning if not given or empty...
            myCurrentTypeID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            const char* const id = myCurrentTypeID.c_str();
            const std::string defType = myTypeCont.knows(myCurrentTypeID) ? myCurrentTypeID : "";
            const int priority = attrs.getOpt<int>(SUMO_ATTR_PRIORITY, id, ok, myTypeCont.getPriority(defType));
            const int numLanes = attrs.getOpt<int>(SUMO_ATTR_NUMLANES, id, ok, myTypeCont.getNumLanes(defType));
            const double speed = attrs.getOpt<double>(SUMO_ATTR_SPEED, id, ok, myTypeCont.getSpeed(defType));
            const std::string allowS = attrs.getOpt<std::string>(SUMO_ATTR_ALLOW, id, ok, "");
            const std::string disallowS = attrs.getOpt<std::string>(SUMO_ATTR_DISALLOW, id, ok, "");
            const bool oneway = attrs.getOpt<bool>(SUMO_ATTR_ONEWAY, id, ok, myTypeCont.getIsOneWay(defType));
            const bool discard = attrs.getOpt<bool>(SUMO_ATTR_DISCARD, id, ok, false);
            const double width = attrs.getOpt<double>(SUMO_ATTR_WIDTH, id, ok, myTypeCont.getWidth(defType));
            const double maxWidth = attrs.getOpt<double>(SUMO_ATTR_MAXWIDTH, id, ok, myTypeCont.getMaxWidth(defType));
            const double minWidth = attrs.getOpt<double>(SUMO_ATTR_MINWIDTH, id, ok, myTypeCont.getMinWidth(defType));
            const double widthResolution = attrs.getOpt<double>(SUMO_ATTR_WIDTHRESOLUTION, id, ok, myTypeCont.getWidthResolution(defType));
            const double sidewalkWidth = attrs.getOpt<double>(SUMO_ATTR_SIDEWALKWIDTH, id, ok, myTypeCont.getSidewalkWidth(defType));
            const double bikeLaneWidth = attrs.getOpt<double>(SUMO_ATTR_BIKELANEWIDTH, id, ok, myTypeCont.getBikeLaneWidth(defType));
            if (!ok) {
                return;
            }
            // build the type
            SVCPermissions permissions = myTypeCont.getPermissions(defType);
            if (allowS != "" || disallowS != "") {
                permissions = parseVehicleClasses(allowS, disallowS);
            }
            myTypeCont.insert(myCurrentTypeID, numLanes, speed, priority, permissions, width, oneway, sidewalkWidth, bikeLaneWidth, widthResolution, maxWidth, minWidth);
            if (discard) {
                myTypeCont.markAsToDiscard(myCurrentTypeID);
            }
            SumoXMLAttr myAttrs[] = {SUMO_ATTR_PRIORITY, SUMO_ATTR_NUMLANES, SUMO_ATTR_SPEED,
                                     SUMO_ATTR_ALLOW, SUMO_ATTR_DISALLOW, SUMO_ATTR_ONEWAY,
                                     SUMO_ATTR_DISCARD, SUMO_ATTR_WIDTH, SUMO_ATTR_SIDEWALKWIDTH, SUMO_ATTR_BIKELANEWIDTH
                                    };
            for (int i = 0; i < 10; i++) {
                if (attrs.hasAttribute(myAttrs[i])) {
                    myTypeCont.markAsSet(myCurrentTypeID, myAttrs[i]);
                }
            }
            break;
        }
        case SUMO_TAG_RESTRICTION: {
            bool ok = true;
            const SUMOVehicleClass svc = getVehicleClassID(attrs.get<std::string>(SUMO_ATTR_VCLASS, myCurrentTypeID.c_str(), ok));
            const double speed = attrs.get<double>(SUMO_ATTR_SPEED, myCurrentTypeID.c_str(), ok);
            if (ok) {
                myTypeCont.addRestriction(myCurrentTypeID, svc, speed);
            }
            break;
        }
        default:
            break;
    }
}


/****************************************************************************/
