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
/// @file    NBFrame.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    09.05.2011
///
// Sets and checks options for netbuild
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include "NBEdgeCont.h"
#include "NBTypeCont.h"
#include "NBNodeCont.h"
#include "NBNode.h"
#include "NBTrafficLightLogicCont.h"
#include "NBDistrictCont.h"
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBFrame
 * @brief Sets and checks options for netbuild
 */
class NBFrame {
public:
    /** @brief Inserts options used by the network converter
     * @param[in] forNetgen Whether netconvert options shall be omitted
     */
    static void fillOptions(bool forNetgen);


    /** @brief Checks set options from the OptionsCont-singleton for being valid
     * @return Whether needed (netbuild) options are set and have proper values
     */
    static bool checkOptions();


};
