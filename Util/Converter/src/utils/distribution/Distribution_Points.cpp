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
/// @file    Distribution_Points.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The description of a distribution by a curve
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <sstream>
#include <iomanip>
#include <utils/common/StdDefs.h>
#include "Distribution_Points.h"


// ===========================================================================
// method definitions
// ===========================================================================
Distribution_Points::Distribution_Points(const std::string& id)
    : Distribution(id) {}


Distribution_Points::~Distribution_Points() {}


double
Distribution_Points::getMax() const {
    assert(getVals().size() > 0);
    return getVals().back();
}


std::string
Distribution_Points::toStr(std::streamsize accuracy) const {
    std::stringstream oss;
    oss << std::setprecision(accuracy);
    const std::vector<double> vals = getVals();
    for (int i = 0; i < (int)vals.size(); i++) {
        if (i > 0) {
            oss << ",";
        }
        oss << vals[i] << ":" << getProbs()[i];
    }
    return "points(" + oss.str() + ")";
}


/****************************************************************************/
