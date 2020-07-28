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
/// @file    SUMOSAXHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// SAX-handler base for SUMO-files
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <utils/xml/GenericSAXHandler.h>
#include "SUMOXMLDefinitions.h"
#include "SUMOSAXHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
SUMOSAXHandler::SUMOSAXHandler(const std::string& file, const std::string& expectedRoot)
    : GenericSAXHandler(SUMOXMLDefinitions::tags, SUMO_TAG_NOTHING,
                        SUMOXMLDefinitions::attrs, SUMO_ATTR_NOTHING, file, expectedRoot) {}


SUMOSAXHandler::~SUMOSAXHandler() {}


/****************************************************************************/
