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
/// @file    GUIContainer.h
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @date    Wed, 01.08.2014
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
#include <microsim/transportables/MSTransportable.h>
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
 * @class GUIContainer
 */
class GUIContainer : public MSTransportable, public GUIGlObject {
public:
    /** @brief Constructor
     */
    GUIContainer(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan);


    /// @brief destructor
    ~GUIContainer();


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
    //@}

    /* @brief set the position of a container while being transported by a vehicle
     * @note This must be called by the vehicle before the call to drawGl */
    void setPositionInVehicle(const GUIBaseVehicle::Seat& pos) {
        myPositionInVehicle = pos;
    }

    /// @name inherited from MSContainer with added locking
    //@{

    /// @brief return the offset from the start of the current edge
    double getEdgePos() const;

    /// @brief return the Network coordinate of the container
    // @note overrides the base method and returns myPositionInVehicle while in driving stage
    Position getPosition() const;

    /// @brief return the current angle of the container
    double getAngle() const;

    /// @brief the time this container spent waiting in seconds
    double getWaitingSeconds() const;

    /// @brief the current speed of the container
    double getSpeed() const;

    //@}


    /**
     * @class GUIContainerPopupMenu
     *
     * A popup-menu for vehicles. In comparison to the normal popup-menu, this one
     *  also allows to trigger further visualisations and to track the vehicle.
     */
    class GUIContainerPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUIContainerPopupMenu)
    public:
        /** @brief Constructor
         * @param[in] app The main window for instantiation of other windows
         * @param[in] parent The parent view for changing it
         * @param[in] o The object of interest
         */
        GUIContainerPopupMenu(GUIMainWindow& app, GUISUMOAbstractView& parent, GUIGlObject& o);

        /// @brief Destructor
        ~GUIContainerPopupMenu();

        /// @brief Called if the plan shall be shown
        long onCmdShowPlan(FXObject*, FXSelector, void*);
        /// @brief Called if the person shall be tracked
        long onCmdStartTrack(FXObject*, FXSelector, void*);
        /// @brief Called if the person shall not be tracked any longer
        long onCmdStopTrack(FXObject*, FXSelector, void*);

    protected:
        /// @brief default constructor needed by FOX
        FOX_CONSTRUCTOR(GUIContainerPopupMenu)

    };



    /// @brief Enabled visualisations, per view
    std::map<GUISUMOAbstractView*, int> myAdditionalVisualizations;




private:
    /// The mutex used to avoid concurrent updates of the vehicle buffer
    mutable FXMutex myLock;

    /// The position of a container while riding a vehicle
    GUIBaseVehicle::Seat myPositionInVehicle;

    /// @brief sets the color according to the currente settings
    void setColor(const GUIVisualizationSettings& s) const;

    /// @brief gets the color value according to the current scheme index
    double getColorValue(const GUIVisualizationSettings& s, int activeScheme) const;

    /// @brief sets the color according to the current scheme index and some vehicle function
    bool setFunctionalColor(int activeScheme) const;

    /// @name drawing helper methods
    /// @{
    void drawAction_drawAsPoly(const GUIVisualizationSettings& s) const;
    void drawAction_drawAsImage(const GUIVisualizationSettings& s) const;
    /// @}
};
