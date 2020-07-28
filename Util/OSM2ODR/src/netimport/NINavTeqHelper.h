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
/// @file    NINavTeqHelper.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Jul 2006
///
// Some parser methods shared around several formats containing NavTeq-Nets
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NINavTeqHelper
 * @brief Some parser methods shared around several formats containing NavTeq-Nets
 *
 * Networks from NavTeq ofte use categories for speed limits and the number of lanes.
 * This class parses such categories and converts them into proper values.
 */
class NINavTeqHelper {
public:
    /** @brief Returns the speed evaluating the given Navteq-description
     *
     * This method tries to parse the speed category into its int-representation
     *  and to determine the speed that is assigned to the category.
     * If either of both steps can not be perfored, a ProcessError is
     *  thrown.
     *
     * @param[in] id The id of the edge (for debug-output)
     * @param[in] speedClassS The string that describes the speed class
     * @return The converted speed (in m/s)
     * @exception ProcessError If the given speed class definition is not a number or if it is not known
     */
    static double getSpeed(const std::string& id,
                           const std::string& speedClassS);


    /** @brief Returns the lane number evaluating the given Navteq-description
     *
     * @param[in] id The id of the edge (for debug-output)
     * @param[in] laneNoS The string that describes the number of lanes
     * @param[in] speed An additional hint for guessing the proper lane number
     * @return The converted lane number
     * @exception ProcessError If the given lane number definition is not a number or if it is not known
     */
    static int getLaneNumber(const std::string& id,
                             const std::string& laneNoS, double speed);


    /** @brief Adds vehicle classes parsing the given list of allowed vehicles
     *
     * Parses the given class-string and sets all set (allowed) vehicle types
     *  into the given edge using "addVehicleClass".
     *
     * @param[in] e The edge to set the parsed vehicle classes into
     * @param[in] classS The string that contains the information whether a vehicle class is allowed
     * @see addVehicleClass
     */
    static void addVehicleClasses(NBEdge& e, const std::string& classS, const SVCPermissions allPermissions, const SVCPermissions defaultPermissions);

    /// @brief same as addVehicleClasses but for version 6+
    static void addVehicleClassesV6(NBEdge& e, const std::string& classS, const SVCPermissions allPermissions, const SVCPermissions defaultPermissions);

private:
    static bool addCommonVehicleClasses(NBEdge& e, const std::string& classS, const int offset);

};
