/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUITransportableControl.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 13.06.2012
///
// GUI-version of the transportable control for building gui persons and containers
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <microsim/transportables/MSTransportableControl.h>


// ===========================================================================
// class definitions
// ===========================================================================
/** @class GUITransportableControl
 * @brief GUI-version of the transportable control for building gui persons and containers
 */
class GUITransportableControl : public MSTransportableControl {
public:
    /// constructor
    GUITransportableControl(const bool isPerson);


    /// destructor
    virtual ~GUITransportableControl();


    /** @brief Builds a new person
     * @param[in] pars The parameter
     * @param[in] vtype The type (reusing vehicle type container here)
     * @param[in] plan This person's plan
     * @param[in] rng The RNG to compute the optional speed deviation
     */
    virtual MSTransportable* buildPerson(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan,
                                         std::mt19937* rng) const;

    /** @brief Builds a new container
    * @param[in] pars The parameter
    * @param[in] vtype The type (reusing vehicle type container here)
    * @param[in] plan This container's plan
    */
    virtual MSTransportable* buildContainer(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan) const;

    /** @brief Returns the list of all known persons by gl-id
     * @param[fill] into The list to fill with vehicle ids
     * @todo Well, what about concurrent modifications?
     */
    void insertIDs(std::vector<GUIGlID>& into);

protected:

    const bool myIsPerson;
};
