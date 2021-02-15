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
/// @file    GUIUserIO.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2006-10-12
///
// Some OS-dependant functions to ease cliboard manipulation
/****************************************************************************/
#include <config.h>

#include "GUIUserIO.h"


// ===========================================================================
// static member definitions
// ===========================================================================
std::string GUIUserIO::clipped = "";


// ===========================================================================
// method definitions
// ===========================================================================
void
GUIUserIO::copyToClipboard(const FXApp& app, const std::string& text) {
    FXDragType types[] = {FXWindow::stringType, FXWindow::textType};
    if (app.getActiveWindow()->acquireClipboard(types, 2)) {
        clipped = text;
    }
}

std::string
GUIUserIO::copyFromClipboard(const FXApp& app) {
    FXString string;
    if (app.getActiveWindow()->getDNDData(FROM_CLIPBOARD, FXWindow::utf8Type, string)) {
        return string.text();
    };
    return string.text();
}


/****************************************************************************/
