/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOSAXAttributesImpl_Xerces.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Fri, 30 Mar 2007
///
// Encapsulated Xerces-SAX-attributes
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <iostream>
#include <xercesc/sax2/Attributes.hpp>
#include <utils/common/SUMOTime.h>
#include <utils/common/StringUtils.h>
#include "SUMOSAXAttributes.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMOSAXAttributesImpl_Xerces
 * @brief Encapsulated Xerces-SAX-attributes
 *
 * @see SUMOSAXAttributes
 */
class SUMOSAXAttributesImpl_Xerces : public SUMOSAXAttributes {
public:
    /** @brief Constructor
     *
     * @param[in] attrs The encapsulated xerces-attributes
     * @param[in] predefinedTags Map of attribute ids to their xerces-representation
     * @param[in] predefinedTagsMML Map of attribute ids to their (readable) string-representation
     */
    SUMOSAXAttributesImpl_Xerces(const XERCES_CPP_NAMESPACE::Attributes& attrs,
                                 const std::vector<XMLCh*>& predefinedTags,
                                 const std::vector<std::string>& predefinedTagsMML,
                                 const std::string& objectType);


    /// @brief Destructor
    virtual ~SUMOSAXAttributesImpl_Xerces();




    /// @name methods for retrieving attribute values
    /// @{

    /** @brief Returns the information whether the named (by its enum-value) attribute is within the current list
     *
     * @param[in] id The id of the searched attribute
     * @return Whether the attribute is within the attributes
     */
    bool hasAttribute(int id) const;


    /**
     * @brief Returns the bool-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2bool.
     *  If the attribute is empty or ==0, TplConvert<XMLCh>::_2bool throws an
     *  EmptyData-exception which is passed.
     * If the value can not be parsed to a bool, TplConvert<XMLCh>::_2bool throws a
     *  BoolFormatException-exception which is passed.
     *
     * @param[in] id The id of the attribute to return the value of
     * @return The attribute's value as a bool, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     * @exception BoolFormatException If the attribute value can not be parsed to a bool
     */
    bool getBool(int id) const;


    /**
     * @brief Returns the int-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2int.
     *  If the attribute is empty or ==0, TplConvert<XMLCh>::_2int throws an
     *  EmptyData-exception which is passed.
     * If the value can not be parsed to an int, TplConvert<XMLCh>::_2int throws a
     *  NumberFormatException-exception which is passed.
     *
     * @param[in] id The id of the attribute to return the value of
     * @return The attribute's value as an int, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     * @exception NumberFormatException If the attribute value can not be parsed to an int
     */
    int getInt(int id) const;


    /**
     * @brief Returns the long-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2long.
     *  If the attribute is empty or ==0, TplConvert<XMLCh>::_2long throws an
     *  EmptyData-exception which is passed.
     * If the value can not be parsed to a long, TplConvert<XMLCh>::_2long throws a
     *  NumberFormatException-exception which is passed.
     *
     * @param[in] id The id of the attribute to return the value of
     * @return The attribute's value as an int, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     * @exception NumberFormatException If the attribute value can not be parsed to an int
     */
    long long int getLong(int id) const;


    /**
     * @brief Returns the string-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2str.
     *  If the attribute is ==0, TplConvert<XMLCh>::_2str throws an
     *  EmptyData-exception which is passed.
     *
     * @param[in] id The id of the attribute to return the value of
     * @return The attribute's value as a string, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     */
    std::string getString(int id) const;

    /**
     * @brief Returns the string-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2strSec.
     *  If the attribute is ==0, TplConvert<XMLCh>::_2strSec returns the default value.
     *
     * @param[in] id The id of the attribute to return the value of
     * @param[in] def The default value to return if the attribute is not in attributes
     * @return The attribute's value as a string, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     */
    std::string getStringSecure(int id,
                                const std::string& def) const;


    /**
     * @brief Returns the double-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2double.
     *  If the attribute is empty or ==0, TplConvert<XMLCh>::_2double throws an
     *  EmptyData-exception which is passed.
     * If the value can not be parsed to a double, TplConvert<XMLCh>::_2double throws a
     *  NumberFormatException-exception which is passed.
     *
     * @param[in] id The id of the attribute to return the value of
     * @return The attribute's value as a float, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     * @exception NumberFormatException If the attribute value can not be parsed to an double
     */
    double getFloat(int id) const;


    /**
     * @brief Returns the information whether the named attribute is within the current list
     */
    bool hasAttribute(const std::string& id) const;


    /**
     * @brief Returns the double-value of the named attribute
     *
     * Tries to retrieve the attribute from the the attribute list. The retrieved
     *  attribute  (which may be 0) is then parsed using TplConvert<XMLCh>::_2double.
     *  If the attribute is empty or ==0, TplConvert<XMLCh>::_2double throws an
     *  EmptyData-exception which is passed.
     * If the value can not be parsed to a double, TplConvert<XMLCh>::_2double throws a
     *  NumberFormatException-exception which is passed.
     *
     * @param[in] id The name of the attribute to return the value of
     * @return The attribute's value as a float, if it could be read and parsed
     * @exception EmptyData If the attribute is not known or the attribute value is an empty string
     * @exception NumberFormatException If the attribute value can not be parsed to an double
     */
    double getFloat(const std::string& id) const;


    /**
     * @brief Returns the string-value of the named (by its enum-value) attribute
     *
     * Tries to retrieve the attribute from the the attribute list.
     *  If the attribute is ==0, TplConvert<XMLCh>::_2strSec returns the default value.
     * @param[in] id The name of the attribute to return the value of
     * @param[in] def The default value to return if the attribute is not in attributes
     * @return The attribute's value as a string, if it could be read and parsed
     */
    std::string getStringSecure(const std::string& id,
                                const std::string& def) const;
    //}


    /**
     * @brief Returns the value of the named attribute
     *
     * Tries to retrieve the attribute from the the attribute list.
     * @return The attribute's value as a SumoXMLEdgeFunc, if it could be read and parsed
     */
    SumoXMLEdgeFunc getEdgeFunc(bool& ok) const;


    /**
     * @brief Returns the value of the named attribute
     *
     * Tries to retrieve the attribute from the the attribute list.
     * @return The attribute's value as a SumoXMLNodeType, if it could be read and parsed
     */
    SumoXMLNodeType getNodeType(bool& ok) const;

    /// @brief returns rightOfWay method
    RightOfWay getRightOfWay(bool& ok) const;

    /// @brief returns fringe type
    FringeType getFringeType(bool& ok) const;

    /**
     * @brief Returns the value of the named attribute
     *
     * Tries to retrieve the attribute from the the attribute list.
     * @return The attribute's value as a RGBColor, if it could be read and parsed
     */
    RGBColor getColor() const;


    /** @brief Tries to read given attribute assuming it is a PositionVector
     *
     * @param[in] attr The id of the attribute to read
     * @return The read value if given and not empty; "" if an error occurred
     */
    PositionVector getShape(int attr) const;


    /** @brief Tries to read given attribute assuming it is a Boundary
     *
     * @param[in] attr The id of the attribute to read
     * @return The read value if given and not empty; "" if an error occurred
     */
    Boundary getBoundary(int attr) const;

    /** @brief Converts the given attribute id into a man readable string
     *
     * Returns a "?" if the attribute is not known.
     *
     * @param[in] attr The id of the attribute to return the name of
     * @return The name of the described attribute
     */
    std::string getName(int attr) const;


    /** @brief Prints all attribute names and values into the given stream
     *
     * @param[in] os The stream to use
     */
    void serialize(std::ostream& os) const;

    /** @brief Retrieves all attribute names
     */
    std::vector<std::string> getAttributeNames() const;

    /// @brief return a new deep-copy attributes object
    SUMOSAXAttributes* clone() const;

private:
    /** @brief Returns Xerces-value of the named attribute
     *
     * It is assumed that this attribute is within the stored attributes.
     * @param[in] id The id of the attribute to retrieve the vale of
     * @return The xerces-value of the attribute
     */
    const XMLCh* getAttributeValueSecure(int id) const;


private:
    /// @brief The encapsulated attributes
    const XERCES_CPP_NAMESPACE::Attributes& myAttrs;

    /// @brief Definition of a map of attribute ids to their xerces-representation
    typedef std::vector<XMLCh*> AttrMap;
    /// @brief Map of attribute ids to their xerces-representation
    const AttrMap& myPredefinedTags;

    /// @brief Map of attribute ids to their (readable) string-representation
    const std::vector<std::string>& myPredefinedTagsMML;

private:
    /// @brief Invalidated copy constructor.
    SUMOSAXAttributesImpl_Xerces(const SUMOSAXAttributesImpl_Xerces& src);

    /// @brief Invalidated assignment operator.
    SUMOSAXAttributesImpl_Xerces& operator=(const SUMOSAXAttributesImpl_Xerces& src);


};
