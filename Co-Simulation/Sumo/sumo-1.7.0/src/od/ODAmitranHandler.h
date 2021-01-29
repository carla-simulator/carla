/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    ODAmitranHandler.h
/// @author  Michael Behrisch
/// @date    27.03.2014
///
// An XML-Handler for Amitran OD matrices
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utility>
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ODMatrix;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ODAmitranHandler
 * @brief An XML-Handler for districts
 *
 * This SUMOSAXHandler parses OD matrices in the Amitran format.
 */
class ODAmitranHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     *
     * Saves the given matrix in order to fill it.
     * @param[in] cont The matrix to fill
     * @param[in] file The file that will be processed
     */
    ODAmitranHandler(ODMatrix& matrix, const std::string& file);


    /// @brief Destructor
    ~ODAmitranHandler();


protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called when an opening-tag occurs
     *
     * Processes district elements via openDistrict, their sinks (via
     *  addSink) and sources (via addSource).
     *
     * @param[in] element The enum of the currently opened element
     * @param[in] attrs Attributes of the currently opened element
     * @exception ProcessError If an error within the parsed file occurs
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);
    //@}


private:
    /// The matrix to add demand to
    ODMatrix& myMatrix;

    std::string myVehicleType;

    SUMOTime myBegin;
    SUMOTime myEnd;
private:
    /// @brief invalidated copy constructor
    ODAmitranHandler(const ODAmitranHandler& s);

    /// @brief invalidated assignment operator
    ODAmitranHandler& operator=(const ODAmitranHandler& s);


};
