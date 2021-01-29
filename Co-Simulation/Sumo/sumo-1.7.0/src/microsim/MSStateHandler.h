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
/// @file    MSStateHandler.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Thu, 13 Dec 2012
///
// Parser and output filter for routes and vehicles state saving and loading
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>
#include "MSRouteHandler.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MESegment;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSStateHandler
 * @brief Parser and output filter for routes and vehicles state saving and loading
 */
class MSStateHandler : public MSRouteHandler {
public:
    /// @brief standard constructor
    MSStateHandler(const std::string& file, const SUMOTime offset, bool onlyReadTime = false);

    /// @brief standard destructor
    virtual ~MSStateHandler();

    /** @brief Saves the current state
     *
     * @param[in] file The file to write the state into
     */
    static void saveState(const std::string& file, SUMOTime step);

    /// @brief get time
    SUMOTime getTime() const {
        return myTime;
    }

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);

    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);
    //@}

    /// Ends the processing of a vehicle
    void closeVehicle();

private:
    /// @brief offset
    const SUMOTime myOffset;

    /// @brief time
    SUMOTime myTime;

    /// @brief segment
    MESegment* mySegment;

    /// @brief current lane being loaded
    MSLane* myCurrentLane;

    /// @brief current link being loaded
    MSLink* myCurrentLink;

    /// @brief que index
    int myQueIndex;

    /// @brief cached attrs (used when loading vehicles or transportables)
    SUMOSAXAttributes* myAttrs;

    /// @brief cached device attrs (used when loading vehicles)
    std::vector<SUMOSAXAttributes*> myDeviceAttrs;

    /// @brief the last object that potentially carries parameters
    Parameterised* myLastParameterised;

    /// @brief whether the handler should abort parsing (via Exception) after parsing the time
    bool myOnlyReadTime;

    /// @brief vehicles that shall be removed when loading state
    std::set<std::string> myVehiclesToRemove;

private:
    /// @brief save the state of random number generators
    static void saveRNGs(OutputDevice& out);

private:
    /// @brief Invalidated copy constructor
    MSStateHandler(const MSStateHandler& s) = delete;

    /// @brief Invalidated assignment operator
    MSStateHandler& operator=(const MSStateHandler& s) = delete;
};
