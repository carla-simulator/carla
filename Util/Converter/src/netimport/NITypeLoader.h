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
/// @file    NITypeLoader.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Perfoms network import
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <xercesc/sax2/SAX2XMLReader.hpp>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class SUMOSAXHandler;
class NBNetBuilder;
class Position;
class PositionVector;



class NITypeLoader {
    /**
     * @class NITypeLoader
     * @brief loads a specificy type of xml file.
     * @note Extra class to simplify import * from netgenerate
     */
public:
    /** loads data from the list of xml-files of certain type */
    static bool load(SUMOSAXHandler* handler, const std::vector<std::string>& files,
                     const std::string& type, const bool stringParse = false);
};
