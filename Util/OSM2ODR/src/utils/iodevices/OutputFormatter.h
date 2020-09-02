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
/// @file    OutputFormatter.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2012
///
// Abstract base class for output formatters
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class Boundary;
class Position;
class PositionVector;
class RGBColor;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class OutputFormatter
 * @brief Abstract base class for output formatters
 *
 * OutputFormatter format XML like output into the output stream.
 *  There are only two implementation at the moment, "normal" XML
 *  and binary XML.
 */
class OutputFormatter {
public:
    /// @brief Destructor
    virtual ~OutputFormatter() { }


    /** @brief Writes an XML header with optional configuration
     *
     * If something has been written (myXMLStack is not empty), nothing
     *  is written and false returned.
     *
     * @param[in] into The output stream to use
     * @param[in] rootElement The root element to use
     * @param[in] attrs Additional attributes to save within the rootElement
     * @todo Check which parameter is used herein
     * @todo Describe what is saved
     */
    virtual bool writeXMLHeader(std::ostream& into, const std::string& rootElement,
                                const std::map<SumoXMLAttr, std::string>& attrs) = 0;


    /** @brief Opens an XML tag
     *
     * An indentation, depending on the current xml-element-stack size, is written followed
     *  by the given xml element ("<" + xmlElement)
     * The xml element is added to the stack, then.
     *
     * @param[in] into The output stream to use
     * @param[in] xmlElement Name of element to open
     * @return The OutputDevice for further processing
     */
    virtual void openTag(std::ostream& into, const std::string& xmlElement) = 0;


    /** @brief Opens an XML tag
     *
     * Helper method which finds the correct string before calling openTag.
     *
     * @param[in] into The output stream to use
     * @param[in] xmlElement Id of the element to open
     */
    virtual void openTag(std::ostream& into, const SumoXMLTag& xmlElement) = 0;


    /** @brief Closes the most recently opened tag and optinally add a comment
     *
     * @param[in] into The output stream to use
     * @return Whether a further element existed in the stack and could be closed
     * @todo it is not verified that the topmost element was closed
     */
    virtual bool closeTag(std::ostream& into, const std::string& comment = "") = 0;

    virtual void writePreformattedTag(std::ostream& into, const std::string& val) = 0;

    virtual void writePadding(std::ostream& into, const std::string& val) = 0;

};
