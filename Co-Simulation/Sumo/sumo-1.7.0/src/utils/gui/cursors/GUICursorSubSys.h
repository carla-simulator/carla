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
/// @file    GUICursorSubSys.h
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2018
///
// A class to manage icons of SUMO
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include "GUICursors.h"


// ===========================================================================
// class definitions
// ===========================================================================
class GUICursorSubSys {
public:
    /**@brief Initiate GUICursorSubSys
     * @param[in] a FOX Toolkit APP
     */
    static void initCursors(FXApp* a);

    /**@brief returns a cursor previously defined in the enum GUICursor
     * @param[in] which GUICursor enum
     */
    static FXCursor* getCursor(GUICursor which);

    /// @brief close GUICursorSubSys
    static void close();

private:
    /// @brief constructor is private because is called by the static function init(FXApp* a)
    GUICursorSubSys(FXApp* a);

    /// @brief destructor
    ~GUICursorSubSys();

    /// @brief instance of GUICursorSubSys
    static GUICursorSubSys* myInstance;

    /// @brief vector with the icons
    FXCursor* myCursors[CURSOR_MAX];
};
