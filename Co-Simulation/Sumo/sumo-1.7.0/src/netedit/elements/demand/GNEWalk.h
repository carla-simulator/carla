/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    GNEWalk.h
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// A class for visualizing walks in Netedit
/****************************************************************************/
#pragma once
#include "GNEDemandElement.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>

// ===========================================================================
// class declarations
// ===========================================================================
class GNEEdge;
class GNEConnection;
class GNEVehicle;

// ===========================================================================
// class definitions
// ===========================================================================

class GNEWalk : public GNEDemandElement, public Parameterised {

public:
    /**@brief parameter constructor for person edge->edge
    * @param[in] viewNet view in which this Walk is placed
    * @param[in] personParent person parent
    * @param[in] fromEdge from edge
    * @param[in] toEdge to edge
    * @param[in] arrivalPosition arrival position on the destination edge
    */
    GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge, double arrivalPosition);

    /**@brief parameter constructor for person edge->busStop
    * @param[in] viewNet view in which this Walk is placed
    * @param[in] personParent person parent
    * @param[in] fromEdge from edge
    * @param[in] toBusStop to busStop
    * @param[in] arrivalPosition arrival position on the destination edge
    */
    GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEAdditional* toBusStop, double arrivalPosition);

    /**@brief parameter constructor for person edge->edge
    * @param[in] viewNet view in which this Walk is placed
    * @param[in] personParent person parent
    * @param[in] fromBusStop from busStop
    * @param[in] toEdge to edge
    * @param[in] arrivalPosition arrival position on the destination edge
    */
    GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEEdge* toEdge, double arrivalPosition);

    /**@brief parameter constructor for person edge->edge
    * @param[in] viewNet view in which this Walk is placed
    * @param[in] personParent person parent
    * @param[in] fromBusStop from busStop
    * @param[in] toBusStop to busStop
    * @param[in] arrivalPosition arrival position on the destination edge
    */
    GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEAdditional* toBusStop, double arrivalPosition);

    /**@brief parameter constructor for person edge->edge
    * @param[in] viewNet view in which this Walk is placed
    * @param[in] personParent person parent
    * @param[in] edges list of edges
    * @param[in] arrivalPosition arrival position on the destination edge
    */
    GNEWalk(GNENet* net, GNEDemandElement* personParent, std::vector<GNEEdge*> edges, double arrivalPosition);

    /**@brief parameter constructor for person edge->edge
    * @param[in] viewNet view in which this Walk is placed
    * @param[in] personParent person parent
    * @param[in] route route
    * @param[in] arrivalPosition arrival position on the destination edge
    */
    GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEDemandElement* route, double arrivalPosition);

    /// @brief destructor
    ~GNEWalk();

    /**@brief writte demand element element into a xml file
     * @param[in] device device in which write parameters of demand element element
     */
    void writeDemandElement(OutputDevice& device) const;

    /// @brief check if current demand element is valid to be writed into XML (by default true, can be reimplemented in children)
    bool isDemandElementValid() const;

    /// @brief return a string with the current demand element problem (by default empty, can be reimplemented in children)
    std::string getDemandElementProblem() const;

    /// @brief fix demand element problem (by default throw an exception, has to be reimplemented in children)
    void fixDemandElementProblem();

    /// @name members and functions relative to elements common to all demand elements
    /// @{
    /// @brief obtain VClass related with this demand element
    SUMOVehicleClass getVClass() const;

    /// @brief get color
    const RGBColor& getColor() const;

    /// @}

    /// @name Functions related with geometry of element
    /// @{
    /// @brief begin geometry movement
    void startGeometryMoving();

    /// @brief end geometry movement
    void endGeometryMoving();

    /**@brief change the position of the element geometry without saving in undoList
     * @param[in] offset Position used for calculate new position of geometry without updating RTree
     */
    void moveGeometry(const Position& offset);

    /**@brief commit geometry changes in the attributes of an element after use of moveGeometry(...)
     * @param[in] undoList The undoList on which to register changes
     */
    void commitGeometryMoving(GNEUndoList* undoList);

    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief compute path
    void computePath();

    /// @brief invalidate path
    void invalidatePath();

    /// @brief Returns position of additional in view
    Position getPositionInView() const;
    /// @}

    /// @name inherited from GUIGlObject
    /// @{

    /**@brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /**@brief Returns the name of the parent object
     * @return This object's parent id
     */
    std::string getParentName() const;

    /**@brief Returns the boundary to which the view shall be centered in order to show the object
     * @return The boundary the object is within
     */
    Boundary getCenteringBoundary() const;

    /// @brief split geometry
    void splitEdgeGeometry(const double splitPosition, const GNENetworkElement* originalElement, const GNENetworkElement* newElement, GNEUndoList* undoList);

    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;

    /**@brief Draws partial object
    * @param[in] s The settings for the current view (may influence drawing)
    * @param[in] lane lane in which draw partial
    * @param[in] drawGeometry flag to enable/disable draw geometry (lines, boxLines, etc.)
    */
    void drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const;

    /**@brief Draws partial object (junction)
     * @param[in] s The settings for the current view (may influence drawing)
     * @param[in] fromLane from GNELane
     * @param[in] toLane to GNELane
     * @param[in] offsetFront offset for drawing element front (needed for selected elements)
     */
    void drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const;
    /// @}

    /// @brief inherited from GNEAttributeCarrier
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

    /* @brief method for setting the attribute and letting the object perform additional changes
    * @param[in] key The attribute key
    * @param[in] value The new value
    * @param[in] undoList The undoList on which to register changes
    * @param[in] net optionally the GNENet to inform about gui updates
    */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /* @brief method for setting the attribute and letting the object perform additional changes
    * @param[in] key The attribute key
    * @param[in] value The new value
    * @param[in] undoList The undoList on which to register changes
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
    /// @brief variable for move walks
    DemandElementMove myWalkMove;

    /// @brief arrival position
    double myArrivalPosition;

private:
    /// @brief method for setting the attribute and nothing else
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    void setEnabledAttribute(const int enabledAttributes);

    /// @brief Invalidated copy constructor.
    GNEWalk(GNEWalk*) = delete;

    /// @brief Invalidated assignment operator.
    GNEWalk& operator=(GNEWalk*) = delete;
};
