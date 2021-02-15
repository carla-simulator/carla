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
/// @file    GNEDataInterval.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A abstract class for data elements
/****************************************************************************/
#pragma once


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <netedit/elements/GNEHierarchicalElement.h>

#include "GNEDataSet.h"

// ===========================================================================
// class declarations
// ===========================================================================

class GNEDataSet;
class GNEGenericData;

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEDataInterval
 * @brief An Element which don't belongs to GNENet but has influency in the simulation
 */
class GNEDataInterval : public GNEHierarchicalElement {

public:
    /**@brief Constructor
     * @param[in] dataSetParent GNEDataSet to which this data interval belongs.
     * @param[in] begin interval begin
     * @param[in] end interval end
     */
    GNEDataInterval(GNEDataSet* dataSetParent, const double begin, const double end);

    /// @brief Destructor
    ~GNEDataInterval();

    /// @brief update generic data child IDs
    void updateGenericDataIDs();

    /// @brief update attribute colors deprecated
    void updateAttributeColors();

    /// @brief all attribute colors
    const GNEDataSet::AttributeColors& getAllAttributeColors() const;

    /// @brief specific attribute colors
    const std::map<SumoXMLTag, GNEDataSet::AttributeColors>& getSpecificAttributeColors() const;

    /// @brief get ID
    const std::string& getID() const;

    /// @brief get GUIGlObject associated with this AttributeCarrier
    GUIGlObject* getGUIGlObject();

    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns element position in view
    Position getPositionInView() const;

    /// @name members and functions relative to write data elements into XML
    /// @{
    /// @brief check if current data element is valid to be writed into XML (by default true, can be reimplemented in children)
    bool isDataIntervalValid() const;

    /// @brief return a string with the current data element problem (by default empty, can be reimplemented in children)
    std::string getDataIntervalProblem() const;

    /// @brief fix data element problem (by default throw an exception, has to be reimplemented in children)
    void fixDataIntervalProblem();
    /// @}

    /// @brief Returns a pointer to GNEDataSet parent
    GNEDataSet* getDataSetParent() const;

    /// @name generic data children
    /// @{

    /// @brief add generic data child
    void addGenericDataChild(GNEGenericData* genericData);

    /// @brief add generic data child
    void removeGenericDataChild(GNEGenericData* genericData);

    /// @brief check if given generic data is child of this data interval
    bool hasGenericDataChild(GNEGenericData* genericData) const;

    /// @brief get generic data children
    const std::vector<GNEGenericData*>& getGenericDataChildren() const;

    /// @}
    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    std::string getAttribute(SumoXMLAttr key) const;

    /* @brief method for getting the Attribute of an XML key in double format (to avoid unnecessary parse<double>(...) for certain attributes)
     * @param[in] key The attribute key
     * @return double with the value associated to key
     */
    double getAttributeDouble(SumoXMLAttr key) const;

    /**@brief method for setting the attribute and letting the object perform data element changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /**@brief method for checking if the key and their conrrespond attribute are valids
     * @param[in] key The attribute key
     * @param[in] value The value asociated to key key
     * @return true if the value is valid, false in other case
     */
    bool isValid(SumoXMLAttr key, const std::string& value);

    /* @brief method for enable attribute
     * @param[in] key The attribute key
     * @param[in] undoList The undoList on which to register changes
     * @note certain attributes can be only enabled, and can produce the disabling of other attributes
     */
    void enableAttribute(SumoXMLAttr key, GNEUndoList* undoList);

    /* @brief method for disable attribute
     * @param[in] key The attribute key
     * @param[in] undoList The undoList on which to register changes
     * @note certain attributes can be only enabled, and can produce the disabling of other attributes
     */
    void disableAttribute(SumoXMLAttr key, GNEUndoList* undoList);

    /* @brief method for check if the value for certain attribute is set
     * @param[in] key The attribute key
     */
    bool isAttributeEnabled(SumoXMLAttr key) const;

    /// @brief get PopPup ID (Used in AC Hierarchy)
    std::string getPopUpID() const;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    std::string getHierarchyName() const;
    /// @}

protected:
    /// @brief all attribute colors
    GNEDataSet::AttributeColors myAllAttributeColors;

    /// @brief specific attribute colors
    std::map<SumoXMLTag, GNEDataSet::AttributeColors> mySpecificAttributeColors;

    /// @brief GNEDataSet parent to which this data interval belongs.
    GNEDataSet* myDataSetParent;

    /// @brief begin interval
    double myBegin;

    /// @brief end interval
    double myEnd;

    /// @brief vector with generic data children
    std::vector<GNEGenericData*> myGenericDataChildren;

private:
    /// @brief method for setting the attribute and nothing else (used in GNEChange_Attribute)
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    void setEnabledAttribute(const int enabledAttributes);

    /// @brief Invalidated copy constructor.
    GNEDataInterval(const GNEDataInterval&) = delete;

    /// @brief Invalidated assignment operator.
    GNEDataInterval& operator=(const GNEDataInterval&) = delete;
};

/****************************************************************************/

