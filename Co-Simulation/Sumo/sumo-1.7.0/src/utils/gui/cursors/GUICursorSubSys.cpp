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
/// @file    GUICursorSubSys.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2018
///
// Helper for cursors loading and usage
/****************************************************************************/
#include <config.h>

#include <fx.h>
#include <cassert>
#include "GUICursors.h"
#include "GUICursorSubSys.h"

/** includes **/

// ===========================================================================
// static member variable definitions
// ===========================================================================

GUICursorSubSys* GUICursorSubSys::myInstance = nullptr;

// ===========================================================================
// member definitions
// ===========================================================================

GUICursorSubSys::GUICursorSubSys(FXApp* a) {
    // already created cursor
    myCursors[SUMOCURSOR_DEFAULT] = a->getDefaultCursor(DEF_ARROW_CURSOR);
    myCursors[SUMOCURSOR_MOVE] = a->getDefaultCursor(DEF_MOVE_CURSOR);

    /*
    myCursors[CURSOR_SUMO] = new FXXPMCursor(a, sumo_icon64_xpm);

    // ... and create them
    for (int i = 0; i < CURSOR_MAX; i++) {
        if (myCursors[i] != nullptr) {
            myCursors[i]->create();
        }
    }
    */
}


GUICursorSubSys::~GUICursorSubSys() {
    /*
    for (int i = 0; i < CURSOR_MAX; i++) {
        delete myCursors[i];
    }
    */
}


void
GUICursorSubSys::initCursors(FXApp* a) {
    assert(myInstance == 0);
    myInstance = new GUICursorSubSys(a);
}


FXCursor*
GUICursorSubSys::getCursor(GUICursor which) {
    return myInstance->myCursors[which];
}


void
GUICursorSubSys::close() {
    delete myInstance;
    myInstance = nullptr;
}


/****************************************************************************/
