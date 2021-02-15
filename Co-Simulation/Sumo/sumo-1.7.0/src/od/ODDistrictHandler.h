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
/// @file    ODDistrictHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// An XML-Handler for districts
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utility>
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ODDistrict;
class ODDistrictCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ODDistrictHandler
 * @brief An XML-Handler for districts
 *
 * This SUMOSAXHandler parses districts and their sinks and sources from
 *  and stores them into a the district container given at initialisation.
 */
class ODDistrictHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     *
     * Saves the given district containe in order to fill it.
     * @param[in] cont The container of districts to fill
     * @param[in] file The file that will be processed
     */
    ODDistrictHandler(ODDistrictCont& cont, const std::string& file);


    /// @brief Destructor
    ~ODDistrictHandler();


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


    /** @brief Called when a closing tag occurs
     *
     * Processes district elements via closeDistrict.
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If an error within the parsed file occurs
     */
    void myEndElement(int element);
    //@}


private:
    /** @brief Begins the parsing of a district
     *
     * Tries to retrieve the id of a district, adds a message to the
     *  error handler if this fails. Otherwise builds a new district
     *  with this id at myCurrentDistrict.
     *
     * @param[in] attrs Attributes of the currently opened element
     */
    void openDistrict(const SUMOSAXAttributes& attrs);


    /** @brief Adds a read source to the current district
     *
     * Tries to get the id and the weight of the currently parsed source
     *  from the attributes using getValues. If the retrieval could be
     *  done without errors (weight>=0), the so retrieved weighted source
     *  is added to myCurrentDistrict using addSource. (getValues checks
     *  whether myCurrentDistrict is valid)
     *
     * @param[in] attrs Attributes of the currently opened element
     * @todo Checking whether myCurrentDistrict is valid through getValues is not quite nice
     */
    void addSource(const SUMOSAXAttributes& attrs);


    /** @brief Adds a read sink to the current district
     *
     * Tries to get the id and the weight of the currently parsed sink
     *  from the attributes using getValues. If the retrieval could be
     *  done without errors (weight>=0), the so retrieved weighted sink
     *  is added to myCurrentDistrict using addSink. (getValues checks
     *  whether myCurrentDistrict is valid)
     *
     * @param[in] attrs Attributes of the currently opened element
     * @todo Checking whether myCurrentDistrict is valid through getValues is not quite nice
     */
    void addSink(const SUMOSAXAttributes& attrs);


    /** @brief Closes the processing of the current district
     *
     * Adds myCurrentDistrict to myContainer.
     */
    void closeDistrict();


    /** @brief Returns the id and weight for a taz/tazSink/tazSource
     *
     * Checks whether myCurrentDistrict (the currently processed
     *  district) is !=0; in this case, both the id and the weight
     *  are parsed. If one of them is missing or the weight is not numerical,
     *  an error is generated and reported to MsgHandler. The "type"-parameter
     *  is used in order to inform the user whether a source or a sink was
     *  processed. In the case of an error, the returned weight is -1.
     *
     * If no error occurs, the correct id and weight are returned.
     *
     * @param[in] attrs Attributes of the currently opened element
     * @param[in] type The type of the currntly processed connection (sink/source)
     * @return The id and the weight of a taz
     */
    std::pair<std::string, double> parseTAZ(const SUMOSAXAttributes& attrs);

private:
    /// The container to add read districts to
    ODDistrictCont& myContainer;

    /// The currently parsed district
    ODDistrict* myCurrentDistrict;


private:
    /// @brief invalidated copy constructor
    ODDistrictHandler(const ODDistrictHandler& s);

    /// @brief invalidated assignment operator
    ODDistrictHandler& operator=(const ODDistrictHandler& s);


};
