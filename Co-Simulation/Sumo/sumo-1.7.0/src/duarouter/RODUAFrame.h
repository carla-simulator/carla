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
/// @file    RODUAFrame.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Sets and checks options for dua-routing
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODUAFrame
 * @brief Sets and checks options for dua-routing
 */
class RODUAFrame {
public:
    /** @brief Inserts options used by duarouter into the OptionsCont-singleton
     *
     * As duarouter shares several options with other routing appplications, the
     *  insertion of these is done via a call to ROFrame::fillOptions.
     *
     * duarouter-specific options are added afterwards via calls to
     *  "addImportOptions" and "addDUAOptions".
     */
    static void fillOptions();


    /** @brief Checks set options from the OptionsCont-singleton for being valid for usage within duarouter
     *
     * Currently, this is done via a call to "ROFrame::checkOptions".
     *
     * @return Whether all needed options are set
     * @todo probably, more things should be checked...
     */
    static bool checkOptions();


protected:
    /** @brief Inserts import options used by duarouter into the OptionsCont-singleton
     */
    static void addImportOptions();


    /** @brief Inserts dua options used by duarouter into the OptionsCont-singleton
     */
    static void addDUAOptions();


};
