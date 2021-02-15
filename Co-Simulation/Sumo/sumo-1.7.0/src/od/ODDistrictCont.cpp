/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    ODDistrictCont.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    Sept 2002
///
// A container for districts
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/NamedObjectCont.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/RandHelper.h>
#include <utils/common/FileHelpers.h>
#include <utils/options/OptionsCont.h>
#include "ODDistrict.h"
#include "ODDistrictHandler.h"
#include "ODDistrictCont.h"



// ===========================================================================
// method definitions
// ===========================================================================
ODDistrictCont::ODDistrictCont() {}


ODDistrictCont::~ODDistrictCont() {}


std::string
ODDistrictCont::getRandomSourceFromDistrict(const std::string& name) const {
    ODDistrict* district = get(name);
    if (district == nullptr) {
        throw InvalidArgument("There is no district '" + name + "'.");
    }
    return district->getRandomSource();
}


std::string
ODDistrictCont::getRandomSinkFromDistrict(const std::string& name) const {
    ODDistrict* district = get(name);
    if (district == nullptr) {
        throw InvalidArgument("There is no district '" + name + "'.");
    }
    return district->getRandomSink();
}


void
ODDistrictCont::loadDistricts(std::vector<std::string> files) {
    for (std::vector<std::string>::iterator i = files.begin(); i != files.end(); ++i) {
        const std::string& districtfile = *i;
        if (!FileHelpers::isReadable(districtfile)) {
            throw ProcessError("Could not access network file '" + districtfile + "' to load.");
        }
        PROGRESS_BEGIN_MESSAGE("Loading districts from '" + districtfile + "'");
        // build the xml-parser and handler
        ODDistrictHandler handler(*this, districtfile);
        if (!XMLSubSys::runParser(handler, districtfile, true)) {
            PROGRESS_FAILED_MESSAGE();
        } else {
            PROGRESS_DONE_MESSAGE();
        }
    }
}


void
ODDistrictCont::makeDistricts(const std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >& districts) {
    for (std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >::const_iterator it = districts.begin(); it != districts.end(); ++it) {
        ODDistrict* current = new ODDistrict(it->first);
        const std::vector<std::string>& sources = it->second.first;
        for (std::vector<std::string>::const_iterator i = sources.begin(); i != sources.end(); ++i) {
            current->addSource(*i, 1.);
        }
        const std::vector<std::string>& sinks = it->second.second;
        for (std::vector<std::string>::const_iterator i = sinks.begin(); i != sinks.end(); ++i) {
            current->addSink(*i, 1.);
        }
        add(current->getID(), current);
    }
}


/****************************************************************************/
