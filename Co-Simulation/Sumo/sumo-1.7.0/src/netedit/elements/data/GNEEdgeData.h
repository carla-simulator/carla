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
/// @file    GNEEdgeData.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// class for edge data
/****************************************************************************/
#pragma once


// ===========================================================================
// included modules
// ===========================================================================

#include "GNEGenericData.h"

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEEdgeData
 * @brief An Element which don't belongs to GNENet but has influency in the simulation
 */
class GNEEdgeData : public GNEGenericData {

public:
    /**@brief Constructor
     * @param[in] dataIntervalParent pointer to data interval parent
     * @param[in] edgeParent poinetr to Edge parent
     * @param[in] parameters parameters map
     */
    GNEEdgeData(GNEDataInterval* dataIntervalParent, GNEEdge* edgeParent, const std::map<std::string, std::string>& parameters);

    /// @brief Destructor
    ~GNEEdgeData();

    /// @brief get edge data color
    const RGBColor& getColor() const;

    /// @brief check if current edge data is visible
    bool isGenericDataVisible() const;

    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns element position in view
    Position getPositionInView() const;

    /// @name members and functions relative to write data sets into XML
    /// @{
    /**@brief writte data set element into a xml file
     * @param[in] device device in which write parameters of data set element
     */
    void writeGenericData(OutputDevice& device) const;

    /// @brief check if current data set is valid to be writed into XML (by default true, can be reimplemented in children)
    bool isGenericDataValid() const;

    /// @brief return a string with the current data set problem (by default empty, can be reimplemented in children)
    std::string getGenericDataProblem() const;

    /// @brief fix data set problem (by default throw an exception, has to be reimplemented in children)
    void fixGenericDataProblem();
    /// @}

    /// @name inherited from GUIGlObject
    /// @{

    /**@brief Draws the object
    * @param[in] s The settings for the current view (may influence drawing)
    * @see GUIGlObject::drawGL
    */
    void drawGL(const GUIVisualizationSettings& s) const;

    /**@brief Draws partial object (lane)
    * @param[in] s The settings for the current view (may influence drawing)
    * @param[in] lane GNELane in which draw partial
    * @param[in] offsetFront offset for drawing element front (needed for selected elements)
    */
    void drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const;

    /**@brief Draws partial object (junction)
    * @param[in] s The settings for the current view (may influence drawing)
    * @param[in] fromLane from GNELane
    * @param[in] toLane to GNELane
    * @param[in] offsetFront offset for drawing element front (needed for selected elements)
    */
    void drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const;

    //// @brief Returns the boundary to which the view shall be centered in order to show the object
    Boundary getCenteringBoundary() const;

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

    /**@brief method for setting the attribute and letting the object perform data set changes
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

private:
    /// @brief method for setting the attribute and nothing else (used in GNEChange_Attribute)
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    void setEnabledAttribute(const int enabledAttributes);

    /// @brief Invalidated copy constructor.
    GNEEdgeData(const GNEEdgeData&) = delete;

    /// @brief Invalidated assignment operator.
    GNEEdgeData& operator=(const GNEEdgeData&) = delete;
};

/****************************************************************************/

