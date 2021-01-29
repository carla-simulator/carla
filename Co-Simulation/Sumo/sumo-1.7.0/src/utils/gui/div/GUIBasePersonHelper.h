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
/// @file    GUIBasePersonHelper.h
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2019
///
// Functions used in GUIPerson and GNEPerson for drawing persons
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>


// ===========================================================================
// class declarations
// ===========================================================================
class GUIVisualizationSettings;
class GUIGlObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIBasePersonHelper
 * @brief A list of functions used for drawing persons in GUI
 */
class GUIBasePersonHelper {
public:

    static void drawAction_drawAsTriangle(const double angle, const double length, const double width);
    static void drawAction_drawAsCircle(const double length, const double width);
    static void drawAction_drawAsPoly(const double angle, const double length, const double width);
    static void drawAction_drawAsImage(const double angle, const double length, const double width, const std::string& file,
                                       const SUMOVehicleShape guiShape, const double exaggeration);
};
