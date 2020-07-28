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
/// @file    NIXMLTypesHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Importer for edge type information stored in XML
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBTypeCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIXMLTypesHandler
 * @brief Importer for edge type information stored in XML
 *
 * This SAX-handler parses edge type information and stores it in the given
 *  container.
 */
class NIXMLTypesHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     *
     * @param[in, filled] tc The type container to fill
     */
    NIXMLTypesHandler(NBTypeCont& tc);

    /// @brief Destructor
    ~NIXMLTypesHandler();

    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag; Parses edge type information
     *
     * Tries to parse a type. If the type can be parsed, it is stored within
     *  "myTypeCont". Otherwise an error is generated. No exception is thrown
     *  so that the parsing continues and further errors can be reported.
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails (not used herein)
     * @note policy is to throw no exception in order to allow further processing
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);
    //@}


private:
    /// @brief The type container to fill
    NBTypeCont& myTypeCont;

    /// @brief The currently parsed type
    std::string myCurrentTypeID;


private:
    /** @brief invalid copy constructor */
    NIXMLTypesHandler(const NIXMLTypesHandler& s);

    /** @brief invalid assignment operator */
    NIXMLTypesHandler& operator=(const NIXMLTypesHandler& s);


};
