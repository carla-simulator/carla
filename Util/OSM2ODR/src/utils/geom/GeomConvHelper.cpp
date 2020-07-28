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
/// @file    GeomConvHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2003
///
// Some helping functions for geometry parsing
/****************************************************************************/
#include <config.h>

#include <string>
#include <sstream>
#include <utils/geom/PositionVector.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include "GeomConvHelper.h"


// ===========================================================================
// method definitions
// ===========================================================================
PositionVector
GeomConvHelper::parseShapeReporting(const std::string& shpdef, const std::string& objecttype,
                                    const char* objectid, bool& ok, bool allowEmpty, bool report) {
    if (shpdef == "") {
        if (!allowEmpty) {
            emitError(report, "Shape", objecttype, objectid, "the shape is empty");
            ok = false;
        }
        return PositionVector();
    }
    StringTokenizer st(shpdef, " ");
    PositionVector shape;
    while (st.hasNext()) {
        StringTokenizer pos(st.next(), ",");
        if (pos.size() != 2 && pos.size() != 3) {
            emitError(report, "Shape", objecttype, objectid, "the position is neither x,y nor x,y,z");
            ok = false;
            return PositionVector();
        }
        try {
            double x = StringUtils::toDouble(pos.next());
            double y = StringUtils::toDouble(pos.next());
            if (pos.size() == 2) {
                shape.push_back(Position(x, y));
            } else {
                double z = StringUtils::toDouble(pos.next());
                shape.push_back(Position(x, y, z));
            }
        } catch (NumberFormatException&) {
            emitError(report, "Shape", objecttype, objectid, "not numeric position entry");
            ok = false;
            return PositionVector();
        } catch (EmptyData&) {
            emitError(report, "Shape", objecttype, objectid, "empty position entry");
            ok = false;
            return PositionVector();
        }
    }
    return shape;
}


Boundary
GeomConvHelper::parseBoundaryReporting(const std::string& def, const std::string& objecttype,
                                       const char* objectid, bool& ok, bool report) {
    StringTokenizer st(def, ",");
    if (st.size() != 4) {
        emitError(report, "Bounding box", objecttype, objectid, "mismatching entry number");
        ok = false;
        return Boundary();
    }
    try {
        double xmin = StringUtils::toDouble(st.next());
        double ymin = StringUtils::toDouble(st.next());
        double xmax = StringUtils::toDouble(st.next());
        double ymax = StringUtils::toDouble(st.next());
        return Boundary(xmin, ymin, xmax, ymax);
    } catch (NumberFormatException&) {
        emitError(report, "Shape", objecttype, objectid, "not numeric entry");
    } catch (EmptyData&) {
        emitError(report, "Shape", objecttype, objectid, "empty entry");
    }
    ok = false;
    return Boundary();
}


void
GeomConvHelper::emitError(bool report, const std::string& what, const std::string& objecttype,
                          const char* objectid, const std::string& desc) {
    if (!report) {
        return;
    }
    std::ostringstream oss;
    oss << what << " of ";
    if (objectid == nullptr) {
        oss << "a(n) ";
    }
    oss << objecttype;
    if (objectid != nullptr) {
        oss << " '" << objectid << "'";
    }
    oss << " is broken: " << desc << ".";
    WRITE_ERROR(oss.str());
}


/****************************************************************************/
