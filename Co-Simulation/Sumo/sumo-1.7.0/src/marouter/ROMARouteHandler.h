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
/// @file    ROMARouteHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Parser and container for routes during their loading
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ODMatrix;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROMARouteHandler
 * @brief Parser and container for routes during their loading
 *
 * ROMARouteHandler transforms vehicles, trips and flows into contributions
 * to an ODMatrix.
 */
class ROMARouteHandler : public SUMOSAXHandler {
public:
    /// standard constructor
    ROMARouteHandler(ODMatrix& matrix);

    /// standard destructor
    virtual ~ROMARouteHandler();

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
    void myStartElement(int element, const SUMOSAXAttributes& attrs);

    void myEndElement(int element);
    //@}

private:
    /// @brief The matrix to fill
    ODMatrix& myMatrix;

    /// @brief The keys for reading taz
    std::vector<std::string> myTazParamKeys;
    /// @brief The current vehicle parameters
    SUMOVehicleParameter* myVehicleParameter;

private:
    /// @brief Invalidated copy constructor
    ROMARouteHandler(const ROMARouteHandler& s);

    /// @brief Invalidated assignment operator
    ROMARouteHandler& operator=(const ROMARouteHandler& s);

};
