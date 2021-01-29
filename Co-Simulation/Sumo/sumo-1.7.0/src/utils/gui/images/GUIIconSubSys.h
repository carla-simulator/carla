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
/// @file    GUIIconSubSys.h
/// @author  Daniel Krajzewicz
/// @date    Jul 2003
///
// A class to manage icons of SUMO
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <map>

#include "GUIIcons.h"


// ===========================================================================
// class definitions
// ===========================================================================
class GUIIconSubSys {
public:
    /**@brief Initiate GUIIconSubSys
     * @param[in] a FOX Toolkit APP
     */
    static void initIcons(FXApp* a);

    /**@brief returns a icon previously defined in the enum GUIIcon
     * @param[in] which GUIIcon enum
     */
    static FXIcon* getIcon(const GUIIcon which);

    /// @brief close GUIIconSubSys
    static void close();

private:
    /// @brief constructor is private because is called by the static function init(FXApp* a)
    GUIIconSubSys(FXApp* a);

    /// @brief destructor
    ~GUIIconSubSys();

    /// @brief instance of GUIIconSubSys
    static GUIIconSubSys* myInstance;

    /// @brief vector with the icons
    std::map<GUIIcon, FXIcon*> myIcons;
};
