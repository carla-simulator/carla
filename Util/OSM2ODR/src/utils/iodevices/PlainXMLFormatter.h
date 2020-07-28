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
/// @file    PlainXMLFormatter.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2012
///
// Output formatter for plain XML output
/****************************************************************************/
#pragma once
#include <config.h>

#include "OutputFormatter.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PlainXMLFormatter
 * @brief Output formatter for plain XML output
 *
 * PlainXMLFormatter format XML like output into the output stream.
 */
class PlainXMLFormatter : public OutputFormatter {
public:
    /// @brief Constructor
    PlainXMLFormatter(const int defaultIndentation = 0);


    /// @brief Destructor
    virtual ~PlainXMLFormatter() { }


    /** @brief Writes an XML header with optional configuration
     *
     * If something has been written (myXMLStack is not empty), nothing
     *  is written and false returned.
     *
     * @param[in] into The output stream to use
     * @param[in] rootElement The root element to use
     * @param[in] attrs Additional attributes to save within the rootElement
     * @todo Describe what is saved
     */
    bool writeXMLHeader(std::ostream& into, const std::string& rootElement,
                        const std::map<SumoXMLAttr, std::string>& attrs);


    /** @brief Writes an XML header with optional configuration
     *
     * If something has been written (myXMLStack is not empty), nothing
     *  is written and false returned.
     *
     * @param[in] into The output stream to use
     * @param[in] rootElement The root element to use
     */
    bool writeHeader(std::ostream& into, const SumoXMLTag& rootElement);


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
    void openTag(std::ostream& into, const std::string& xmlElement);


    /** @brief Opens an XML tag
     *
     * Helper method which finds the correct string before calling openTag.
     *
     * @param[in] into The output stream to use
     * @param[in] xmlElement Id of the element to open
     */
    void openTag(std::ostream& into, const SumoXMLTag& xmlElement);


    /** @brief Closes the most recently opened tag
     *
     * @param[in] into The output stream to use
     * @return Whether a further element existed in the stack and could be closed
     * @todo it is not verified that the topmost element was closed
     */
    bool closeTag(std::ostream& into, const std::string& comment = "");


    /** @brief writes a preformatted tag to the device but ensures that any
     * pending tags are closed
     * @param[in] into The output stream to use
     * @param[in] val The preformatted data
     */
    void writePreformattedTag(std::ostream& into, const std::string& val);

    /** @brief writes arbitrary padding
     */
    void writePadding(std::ostream& into, const std::string& val);


    /** @brief writes an arbitrary attribute
     *
     * @param[in] into The output stream to use
     * @param[in] attr The attribute (name)
     * @param[in] val The attribute value
     */
    template <class T>
    static void writeAttr(std::ostream& into, const std::string& attr, const T& val) {
        into << " " << attr << "=\"" << toString(val, into.precision()) << "\"";
    }


    /** @brief writes a named attribute
     *
     * @param[in] into The output stream to use
     * @param[in] attr The attribute (name)
     * @param[in] val The attribute value
     */
    template <class T>
    static void writeAttr(std::ostream& into, const SumoXMLAttr attr, const T& val) {
        into << " " << toString(attr) << "=\"" << toString(val, into.precision()) << "\"";
    }


private:
    /// @brief The stack of begun xml elements
    std::vector<std::string> myXMLStack;

    /// @brief The initial indentation level
    int myDefaultIndentation;

    /// @brief whether a closing ">" might be missing
    bool myHavePendingOpener;
};
