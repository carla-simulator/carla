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
/// @file    NIXMLShapeHandler.h
/// @author  Jakob Erdmann
/// @date    Sat, 28 Jul 2018
///
// Importer for static public transport information
/****************************************************************************/
#pragma once

#include <config.h>

#include <string>
#include <utils/shapes/ShapeHandler.h>
// ===========================================================================
// class declarations
// ===========================================================================
class NBEdgeCont;
class Position;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class NBShapeHandler
 * @brief The XML-Handler for shapes loading network loading
 *
 * This subclasses ShapeHandler with MSLane specific function
 */
class NIXMLShapeHandler : public ShapeHandler {
public:
    NIXMLShapeHandler(ShapeContainer& sc, const NBEdgeCont& ec);

    /// @brief Destructor
    virtual ~NIXMLShapeHandler() {}

    Position getLanePos(const std::string& poiID, const std::string& laneID, double lanePos, double lanePosLat);

    virtual bool addLanePosParams() {
        return true;
    }

    const NBEdgeCont& myEdgeCont;
};
