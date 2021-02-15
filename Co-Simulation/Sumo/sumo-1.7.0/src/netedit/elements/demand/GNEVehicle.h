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
/// @file    GNEVehicle.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// Representation of vehicles in NETEDIT
/****************************************************************************/
#pragma once
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>

#include "GNEDemandElement.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEVehicle
 */
class GNEVehicle : public GNEDemandElement, public SUMOVehicleParameter {

public:
    /// @brief class used in GUIGLObjectPopupMenu for single vehicle transformations
    class GNESingleVehiclePopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GNESingleVehiclePopupMenu)

    public:
        /** @brief Constructor
         * @param[in] vehicle GNEVehicle to be transformed
         * @param[in] app The main window for instantiation of other windows
         * @param[in] parent The parent view for changing it
         */
        GNESingleVehiclePopupMenu(GNEVehicle* vehicle, GUIMainWindow& app, GUISUMOAbstractView& parent);

        /// @brief Destructor
        ~GNESingleVehiclePopupMenu();

        /// @brief Called to transform the current vehicle to another vehicle type
        long onCmdTransform(FXObject* obj, FXSelector, void*);

    protected:
        /// @brief default constructor needed by FOX
        FOX_CONSTRUCTOR(GNESingleVehiclePopupMenu);

    private:
        /// @brief current vehicle
        GNEVehicle* myVehicle;

        /// @brief menu command for transform to vehicle
        FXMenuCommand* myTransformToVehicle;

        /// @brief menu command for transform to vehicle with an embedded route
        FXMenuCommand* myTransformToVehicleWithEmbeddedRoute;

        /// @brief menu command for transform to route flow
        FXMenuCommand* myTransformToRouteFlow;

        /// @brief menu command for transform to route flow with an embedded route
        FXMenuCommand* myTransformToRouteFlowWithEmbeddedRoute;

        /// @brief menu command for transform to trip
        FXMenuCommand* myTransformToTrip;

        /// @brief menu command for transform to flow
        FXMenuCommand* myTransformToFlow;
    };

    /// @brief class used in GUIGLObjectPopupMenu for single vehicle transformations
    class GNESelectedVehiclesPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GNESelectedVehiclesPopupMenu)

    public:
        /** @brief Constructor
         * @param[in] vehicle clicked GNEVehicle
         * @param[in] selectedVehicle vector with selected GNEVehicle
         * @param[in] app The main window for instantiation of other windows
         * @param[in] parent The parent view for changing it
         */
        GNESelectedVehiclesPopupMenu(GNEVehicle* vehicle, const std::vector<GNEVehicle*>& selectedVehicle, GUIMainWindow& app, GUISUMOAbstractView& parent);

        /// @brief Destructor
        ~GNESelectedVehiclesPopupMenu();

        /// @brief Called to transform the current vehicle to another vehicle type
        long onCmdTransform(FXObject* obj, FXSelector, void*);

    protected:
        /// @brief default constructor needed by FOX
        FOX_CONSTRUCTOR(GNESelectedVehiclesPopupMenu);

    private:
        /// @brief current selected vehicles
        std::vector<GNEVehicle*> mySelectedVehicles;

        /// @brief tag of clicked vehicle
        SumoXMLTag myVehicleTag;

        /// @brief menu command for transform to vehicle
        FXMenuCommand* myTransformToVehicle;

        /// @brief menu command for transform to vehicle with an embedded route
        FXMenuCommand* myTransformToVehicleWithEmbeddedRoute;

        /// @brief menu command for transform to route flow
        FXMenuCommand* myTransformToRouteFlow;

        /// @brief menu command for transform to route flow with an embedded route
        FXMenuCommand* myTransformToRouteFlowWithEmbeddedRoute;

        /// @brief menu command for transform to trip
        FXMenuCommand* myTransformToTrip;

        /// @brief menu command for transform to flow
        FXMenuCommand* myTransformToFlow;

        /// @brief menu command for transform all selected vehicles to vehicle
        FXMenuCommand* myTransformAllVehiclesToVehicle;

        /// @brief menu command for transform all selected vehicles to vehicle with an embedded route
        FXMenuCommand* myTransformAllVehiclesToVehicleWithEmbeddedRoute;

        /// @brief menu command for transform all selected vehicles to route flow
        FXMenuCommand* myTransformAllVehiclesToRouteFlow;

        /// @brief menu command for transform all selected vehicles to route flow with an embedded route
        FXMenuCommand* myTransformAllVehiclesToRouteFlowWithEmbeddedRoute;

        /// @brief menu command for transform all selected vehicles to trip
        FXMenuCommand* myTransformAllVehiclesToTrip;

        /// @brief menu command for transform all selected vehicles to flow
        FXMenuCommand* myTransformAllVehiclesToFlow;
    };

    /// @brief default constructor for vehicles and routeFlows without embedded routes
    GNEVehicle(SumoXMLTag tag, GNENet* net, const std::string& vehicleID, GNEDemandElement* vehicleType, GNEDemandElement* route);

    /// @brief parameter constructor for vehicles and routeFlows without embedded routes
    GNEVehicle(GNENet* net, GNEDemandElement* vehicleType, GNEDemandElement* route, const SUMOVehicleParameter& vehicleParameters);

    /// @brief parameter constructor for vehicles and routeFlows with embedded routes (note: After creation create immediately a embedded route referencing this vehicle)
    GNEVehicle(GNENet* net, GNEDemandElement* vehicleType, const SUMOVehicleParameter& vehicleParameters);

    /// @brief default constructor for trips and Flows
    GNEVehicle(SumoXMLTag tag, GNENet* net, const std::string& vehicleID, GNEDemandElement* vehicleType, GNEEdge* fromEdge, GNEEdge* toEdge, const std::vector<GNEEdge*>& via);

    /// @brief parameter constructor for trips and Flows
    GNEVehicle(GNENet* net, GNEDemandElement* vehicleType, GNEEdge* fromEdge, GNEEdge* toEdge, const std::vector<GNEEdge*>& via, const SUMOVehicleParameter& vehicleParameters);

    /// @brief destructor
    ~GNEVehicle();

    /**@brief get begin time of demand element
     * @note: used by demand elements of type "Vehicle", and it has to be implemented as children
     * @throw invalid argument if demand element doesn't has a begin time
     */
    std::string getBegin() const;

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
    /// @brief start geometry movement
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

    /// @brief Returns position of demand element in view
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

    /* @brief method for setting the attribute and letting the object perform demand element changes
    * @param[in] key The attribute key
    * @param[in] value The new value
    * @param[in] undoList The undoList on which to register changes
    * @param[in] net optionally the GNENet to inform about gui updates
    */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /* @brief method for setting the attribute and letting the object perform demand element changes
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
    /// @brief sets the color according to the currente settings
    void setColor(const GUIVisualizationSettings& s) const;

private:
    /// @brief method for setting the attribute and nothing else
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief method for enabling the attribute and nothing else (used in GNEChange_EnableAttribute)
    void setEnabledAttribute(const int enabledAttributes);

    /// @brief draw stack label
    void drawStackLabel(const GUIVisualizationSettings& s, const Position& vehiclePosition, const double vehicleRotation, const double width, const double length) const;

    /// @brief draw flow label
    void drawFlowLabel(const GUIVisualizationSettings& s, const Position& vehiclePosition, const double vehicleRotation, const double width, const double length) const;

    /// @brief Invalidated copy constructor.
    GNEVehicle(const GNEVehicle&) = delete;

    /// @brief Invalidated assignment operator
    GNEVehicle& operator=(const GNEVehicle&) = delete;
};
