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
/// @file    GUIManipulator.cpp
/// @author  Daniel Krajzewicz
/// @date    Jun 2004
///
// Abstract GUI manipulation class
/****************************************************************************/
#include <config.h>

#include <fx.h>
#include <string>
#include <gui/GUIApplicationWindow.h>
#include "GUIManipulator.h"
#include <gui/GUIGlobals.h>
#include <utils/gui/div/GUIDesigns.h>


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXIMPLEMENT(GUIManipulator, FXDialogBox, NULL, 0)


// ===========================================================================
// method definitions
// ===========================================================================
GUIManipulator::GUIManipulator(GUIMainWindow& app, const std::string& name, int xpos, int ypos) :
    FXDialogBox(&app, name.c_str(), GUIDesignDialogBox, xpos, ypos, 0, 0) {}


GUIManipulator::~GUIManipulator() {}


/****************************************************************************/
