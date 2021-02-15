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
/// @file    GUIPerson.h
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
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/common/RGBColor.h>
#include <microsim/transportables/MSPerson.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/settings/GUIPropertySchemeStorage.h>
#include "GUIBaseVehicle.h"


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
 * @class GUIPerson
 */
class GUIPerson : public MSPerson, public GUIGlObject {

public:
    /// @brief Constructor
    GUIPerson(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan, const double speedFactor);

    /// @brief destructor
    ~GUIPerson();

    /// @name inherited from GUIGlObject
    /// @{
    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns an own type parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     */
    GUIParameterTableWindow* getTypeParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;

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
    //* @}

    /* @brief proceeds to the next step of the route,
     * @return Whether the transportables plan continues  */
    bool proceed(MSNet* net, SUMOTime time, const bool vehicleArrived = false);

    /* @brief set the position of a person while riding in a vehicle
     * @note This must be called by the vehicle before the call to drawGl */
    void setPositionInVehicle(const GUIBaseVehicle::Seat& pos);

    /// @name inherited from MSPerson with added locking
    /// @{
    /// @brief return the offset from the start of the current edge
    double getEdgePos() const;

    /// @brief return the Network coordinate of the person
    // @note overrides the base method and returns myPositionInVehicle while in driving stage
    Position getPosition() const;

    /// @brief return the Network coordinate of the person (only for drawing centering and tracking)
    Position getGUIPosition() const;

    /// @brief return the angle of the person (only for drawing centering and tracking)
    double getGUIAngle() const;

    /// @brief return the current angle of the person
    double getNaviDegree() const;

    /// @brief the time this person spent waiting in seconds
    double getWaitingSeconds() const;

    /// @brief the current speed of the person
    double getSpeed() const;

    /// @brief get stage index description
    std::string getStageIndexDescription() const;

    /// @brief get edge ID
    std::string getEdgeID() const;

    /// @brief ger from edge ID
    std::string getFromEdgeID() const;

    /// @brief get destination edge ID
    std::string getDestinationEdgeID() const;

    /// @brief get stage arrival position
    double getStageArrivalPos() const;

    //@}

    /// @brief whether this person is selected in the GUI
    bool isSelected() const;

    /**
     * @class GUIPersonPopupMenu
     *
     * A popup-menu for vehicles. In comparison to the normal popup-menu, this one
     *  also allows to trigger further visualisations and to track the vehicle.
     */
    class GUIPersonPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUIPersonPopupMenu)

    public:
        /** @brief Constructor
         * @param[in] app The main window for instantiation of other windows
         * @param[in] parent The parent view for changing it
         * @param[in] o The object of interest
         */
        GUIPersonPopupMenu(GUIMainWindow& app, GUISUMOAbstractView& parent, GUIGlObject& o);

        /// @brief Destructor
        ~GUIPersonPopupMenu();

        /// @brief Called if the current route of the person shall be shown
        long onCmdShowCurrentRoute(FXObject*, FXSelector, void*);

        /// @brief Called if the current route of the person shall be hidden
        long onCmdHideCurrentRoute(FXObject*, FXSelector, void*);

        /// @brief Called if the walkingarea path of the person shall be shown
        long onCmdShowWalkingareaPath(FXObject*, FXSelector, void*);

        /// @brief Called if the walkingarea path of the person shall be hidden
        long onCmdHideWalkingareaPath(FXObject*, FXSelector, void*);

        /// @brief Called if the plan shall be shown
        long onCmdShowPlan(FXObject*, FXSelector, void*);

        /// @brief Called if the person shall be tracked
        long onCmdStartTrack(FXObject*, FXSelector, void*);

        /// @brief Called if the person shall not be tracked any longer
        long onCmdStopTrack(FXObject*, FXSelector, void*);

        /// @brief Called when removing the person
        long onCmdRemoveObject(FXObject*, FXSelector, void*);

    protected:
        /// @brief default constructor needed by FOX
        FOX_CONSTRUCTOR(GUIPersonPopupMenu)
    };

    /// @brief Additional visualisation feature ids
    enum VisualisationFeatures {
        /// @brief show the current walkingarea path
        VO_SHOW_WALKINGAREA_PATH = 1,
        /// @brief show persons's current route
        VO_SHOW_ROUTE = 2,
        /// @brief track person
        VO_TRACKED = 8
    };

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

private:
    /// @brief sets the color according to the currente settings
    void setColor(const GUIVisualizationSettings& s) const;

    /// @brief gets the color value according to the current scheme index
    double getColorValue(const GUIVisualizationSettings& s, int activeScheme) const;

    /// @brief sets the color according to the current scheme index and some vehicle function
    bool setFunctionalColor(int activeScheme) const;

    /// @brief draw walking area path
    void drawAction_drawWalkingareaPath(const GUIVisualizationSettings& s) const;

    /// @brief whether the person is jammed as defined by the current pedestrian model
    bool isJammed() const;

    /// The mutex used to avoid concurrent updates of the vehicle buffer
    mutable FXMutex myLock;

    /// The position of a person while riding a vehicle
    GUIBaseVehicle::Seat myPositionInVehicle;

    /// @brief Enabled visualisations, per view
    std::map<GUISUMOAbstractView*, int> myAdditionalVisualizations;
};
