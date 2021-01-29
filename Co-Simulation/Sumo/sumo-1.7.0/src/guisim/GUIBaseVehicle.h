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
/// @file    GUIBaseVehicle.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A MSVehicle extended by some values for usage within the gui
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <string>
#include <fx.h>
#include <utils/common/RGBColor.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/settings/GUIPropertySchemeStorage.h>
#include <microsim/MSVehicle.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUISUMOAbstractView;
class GUIGLObjectPopupMenu;
class MSDevice_Vehroutes;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIBaseVehicle
 * @brief A MSVehicle extended by some values for usage within the gui
 *
 * A visualisable MSVehicle. Extended by the possibility to retrieve names
 * of all available vehicles (static) and the possibility to retrieve the
 * color of the vehicle which is available in different forms allowing an
 * easier recognition of done actions such as lane changing.
 */
class GUIBaseVehicle : public GUIGlObject {
public:

    GUIBaseVehicle(MSBaseVehicle& vehicle);

    /// @brief destructor
    ~GUIBaseVehicle();

    struct Seat {
        Seat(): pos(Position::INVALID), angle(0) {}

        Seat(const Position& _pos, double _angle):
            pos(_pos), angle(_angle) {}

        Position pos;
        double angle;
    };
    typedef std::vector<Seat> Seats;

    /** @brief Return current position (x/y, cartesian)
     *
     * If the vehicle's myLane is 0, Position::INVALID.
     * @param[in] offset optional offset in longitudinal direction
     * @return The current position (in cartesian coordinates)
     * @see myLane
     */
    virtual Position getPosition(const double offset = 0) const = 0;

    /** @brief Returns the vehicle's direction in radians
     * @return The vehicle's current angle
     */
    virtual double getAngle() const = 0;

    /// @brief return the current angle in navigational degrees
    double getNaviDegree() const {
        return GeomHelper::naviDegree(getAngle());
    }

    /// @brief gets the color value according to the current scheme index
    virtual double getColorValue(const GUIVisualizationSettings& s, int activeScheme) const = 0;

    /// @brief draws the given guiShape with distinct carriages/modules
    virtual void drawAction_drawCarriageClass(const GUIVisualizationSettings& s, bool asImage) const = 0;

    /** @brief Returns the time since the last lane change in seconds
     * @see MSVehicle::myLastLaneChangeOffset
     * @return The time since the last lane change in seconds
     */
    virtual double getLastLaneChangeOffset() const = 0;

    /** @brief Draws the route
     * @param[in] r The route to draw
     */
    virtual void drawRouteHelper(const GUIVisualizationSettings& s, const MSRoute& r, bool future, bool noLoop, const RGBColor& col) const = 0;

    /// @brief retrieve information about the current stop state
    virtual std::string getStopInfo() const = 0;

    /// @brief adds the blocking foes to the current selection
    virtual void selectBlockingFoes() const = 0;

    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    virtual GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent) = 0;

    /** @brief Returns an own type parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     */
    virtual GUIParameterTableWindow* getTypeParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent) = 0;

    virtual void drawAction_drawVehicleBlinker(double /*length*/) const {}
    virtual void drawAction_drawVehicleBrakeLight(double length, bool onlyOne = false) const {
        UNUSED_PARAMETER(length);
        UNUSED_PARAMETER(onlyOne);
    }
    virtual void drawAction_drawLinkItems(const GUIVisualizationSettings& /*s*/) const {}
    virtual void drawAction_drawPersonsAndContainers(const GUIVisualizationSettings& s) const;
    /** @brief Draws the vehicle's best lanes */
    virtual void drawBestLanes() const {};
    virtual void drawAction_drawVehicleBlueLight() const {}





    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /// @brief notify object about popup menu removal
    void removedPopupMenu();

    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;

    /// @brief Returns the value for generic parameter 'name' or ''
    const std::string getOptionalName() const;

    /** @brief Draws the object on the specified position with the specified angle
     * @param[in] s The settings for the current view (may influence drawing)
     * @param[in] pos The position to draw the vehicle on
     * @param[in] angle The drawing angle of the vehicle
     */
    void drawOnPos(const GUIVisualizationSettings& s, const Position& pos, const double angle) const;


    /** @brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;


    /** @brief Draws additionally triggered visualisations
     * @param[in] parent The view
     * @param[in] s The settings for the current view (may influence drawing)
     */
    virtual void drawGLAdditional(GUISUMOAbstractView* const parent, const GUIVisualizationSettings& s) const;
    //@}



    /// @name Additional visualisations
    /// @{

    /** @brief Returns whether the named feature is enabled in the given view
     * @param[in] parent The view for which the feature may be enabled
     * @param[in] which The visualisation feature
     * @return see comment
     */
    bool hasActiveAddVisualisation(GUISUMOAbstractView* const parent, int which) const;


    /** @brief Adds the named visualisation feature to the given view
     * @param[in] parent The view for which the feature shall be enabled
     * @param[in] which The visualisation feature to enable
     * @see GUISUMOAbstractView::addAdditionalGLVisualisation
     */
    void addActiveAddVisualisation(GUISUMOAbstractView* const parent, int which);


    /** @brief Adds the named visualisation feature to the given view
     * @param[in] parent The view for which the feature shall be enabled
     * @param[in] which The visualisation feature to enable
     * @see GUISUMOAbstractView::removeAdditionalGLVisualisation
     */
    void removeActiveAddVisualisation(GUISUMOAbstractView* const parent, int which);
    /// @}

    /// @brief return the number of passengers
    int getNumPassengers() const;

    /// @brief return the number of passengers
    int getNumContainers() const;

    /// @brief lists equipped device (types) for the current vehicle
    std::string getDeviceDescription();

    /**
     * @class GUIBaseVehiclePopupMenu
     *
     * A popup-menu for vehicles. In comparison to the normal popup-menu, this one
     *  also allows to trigger further visualisations and to track the vehicle.
     */
    class GUIBaseVehiclePopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUIBaseVehiclePopupMenu)
    public:
        /** @brief Constructor
         * @param[in] app The main window for instantiation of other windows
         * @param[in] parent The parent view for changing it
         * @param[in] o The object of interest
         */
        GUIBaseVehiclePopupMenu(GUIMainWindow& app, GUISUMOAbstractView& parent, GUIGlObject& o);

        /// @brief Destructor
        ~GUIBaseVehiclePopupMenu();

        /// @brief Called if all routes of the vehicle shall be shown
        long onCmdShowAllRoutes(FXObject*, FXSelector, void*);
        /// @brief Called if all routes of the vehicle shall be hidden
        long onCmdHideAllRoutes(FXObject*, FXSelector, void*);
        /// @brief Called if the current route of the vehicle shall be shown
        long onCmdShowCurrentRoute(FXObject*, FXSelector, void*);
        /// @brief Called if the current route of the vehicle shall be hidden
        long onCmdHideCurrentRoute(FXObject*, FXSelector, void*);
        /// @brief Called if the current route of the vehicle shall be shown
        long onCmdShowFutureRoute(FXObject*, FXSelector, void*);
        /// @brief Called if the current route of the vehicle shall be hidden
        long onCmdHideFutureRoute(FXObject*, FXSelector, void*);
        /// @brief Called if the current route of the vehicle shall be shown
        long onCmdShowRouteNoLoops(FXObject*, FXSelector, void*);
        /// @brief Called if the current route of the vehicle shall be hidden
        long onCmdHideRouteNoLoops(FXObject*, FXSelector, void*);
        /// @brief Called if the vehicle's best lanes shall be shown
        long onCmdShowBestLanes(FXObject*, FXSelector, void*);
        /// @brief Called if the vehicle's best lanes shall be hidden
        long onCmdHideBestLanes(FXObject*, FXSelector, void*);
        /// @brief Called if the vehicle shall be tracked
        long onCmdStartTrack(FXObject*, FXSelector, void*);
        /// @brief Called if the current shall not be tracked any longer
        long onCmdStopTrack(FXObject*, FXSelector, void*);
        /// @brief Called if all routes of the vehicle shall be shown
        long onCmdShowLFLinkItems(FXObject*, FXSelector, void*);
        /// @brief Called if all routes of the vehicle shall be hidden
        long onCmdHideLFLinkItems(FXObject*, FXSelector, void*);
        /// @brief Called when show a vehicles foes
        long onCmdShowFoes(FXObject*, FXSelector, void*);
        /// @brief Called when removing the vehicle
        long onCmdRemoveObject(FXObject*, FXSelector, void*);
        /// @brief Called when toggling stop state
        long onCmdToggleStop(FXObject*, FXSelector, void*);

    protected:
        FOX_CONSTRUCTOR(GUIBaseVehiclePopupMenu)

    };


    /// @name Additional visualisations
    /// @{

    /** @brief Additional visualisation feature ids
     */
    enum VisualisationFeatures {
        /// @brief show vehicle's best lanes
        VO_SHOW_BEST_LANES = 1,
        /// @brief show vehicle's current route
        VO_SHOW_ROUTE = 2,
        /// @brief show all vehicle's routes
        VO_SHOW_ALL_ROUTES = 4,
        /// @brief LFLinkItems
        VO_SHOW_LFLINKITEMS = 8,
        /// @brief draw vehicle outside the road network
        VO_DRAW_OUTSIDE_NETWORK = 16,
        /// @brief show vehicle's current continued from the current position
        VO_SHOW_FUTURE_ROUTE = 32,
        /// @brief show vehicle's routes without loops
        VO_SHOW_ROUTE_NOLOOP = 64
    };

    /// @brief Enabled visualisations, per view
    std::map<GUISUMOAbstractView*, int> myAdditionalVisualizations;


    /** @brief Chooses the route to draw and draws it, darkening it as given
     * @param[in] s The visualisation settings, needed to determine the vehicle's color
     * @param[in] routeNo The route to show (0: the current, >0: prior)
     * @param[in] darken The amount to darken the route by
     */
    void drawRoute(const GUIVisualizationSettings& s, int routeNo, double darken, bool future = false, bool noLoop = false) const;


    /// @}

    /// @brief sets the color according to the current scheme index and some vehicle function
    static bool setFunctionalColor(int activeScheme, const MSBaseVehicle* veh, RGBColor& col);

protected:

    /// @brief sets the color according to the currente settings
    RGBColor setColor(const GUIVisualizationSettings& s) const;

    /// @brief returns the seat position for the person with the given index
    const Seat& getSeatPosition(int personIndex) const;
    const Seat& getContainerPosition(int containerIndex) const;

    static void drawLinkItem(const Position& pos, SUMOTime arrivalTime, SUMOTime leaveTime, double exagerate);

    /// @brief A shortcut to myVehicle.myType
    inline const MSVehicleType& getVType() const {
        return myVehicle.getVehicleType();
    }

    /// @brief draw vehicle body and return whether carriages are being drawn
    bool drawAction_drawVehicleAsPolyWithCarriagges(const GUIVisualizationSettings& s, bool asImage = false) const;

    /// @brief add seats to mySeatPositions and update requiredSeats
    void computeSeats(const Position& front, const Position& back, double seatOffset, int maxSeats, double exaggeration, int& requiredSeats, Seats& into) const;


protected:
    /// The mutex used to avoid concurrent updates of the vehicle buffer
    mutable FXMutex myLock;

    /// @brief positions of seats in the vehicle (updated at every drawing step)
    mutable Seats mySeatPositions;
    mutable Seats myContainerPositions;

private:
    /// @brief The vehicle to which all calls should be delegated
    MSBaseVehicle& myVehicle;

    MSDevice_Vehroutes* myRoutes;

    /// @brief current popup (to clean up in destructor). GUIBaseVehicle is not responsible for removal
    GUIGLObjectPopupMenu* myPopup;

};
