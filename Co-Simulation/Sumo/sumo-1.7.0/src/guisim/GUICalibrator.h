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
/// @file    GUICalibrator.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 26.04.2004
///
// Changes flow and speed on a set of lanes (gui version)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <microsim/trigger/MSCalibrator.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <gui/GUIManipulator.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUICalibrator
 * @brief Changes the speed allowed on a set of lanes (gui version)
 *
 * This is the gui-version of the MSCalibrator-object
 */
class GUICalibrator : public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
     * @param[in] calibrator MSCalibrator or METriggeredCalibrator to be wrapped
     */
    GUICalibrator(MSCalibrator* calibrator);

    /** destructor */
    ~GUICalibrator();



    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
     *
     * @param[in] app The application needed to build the popup-menu
     * @param[in] parent The parent window needed to build the popup-menu
     * @return The built popup-menu
     * @see GUIGlObject::getPopUpMenu
     */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app,
                                       GUISUMOAbstractView& parent);


    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app,
            GUISUMOAbstractView& parent);


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
    //@}



    GUIManipulator* openManipulator(GUIMainWindow& app,
                                    GUISUMOAbstractView& parent);

public:
    class GUICalibratorPopupMenu : public GUIGLObjectPopupMenu {
        FXDECLARE(GUICalibratorPopupMenu)
    public:

        GUICalibratorPopupMenu(GUIMainWindow& app,
                               GUISUMOAbstractView& parent, GUIGlObject& o);

        ~GUICalibratorPopupMenu();

        /** @brief Called if the object's manipulator shall be shown */
        long onCmdOpenManip(FXObject*, FXSelector, void*);

    protected:
        GUICalibratorPopupMenu() { }

    };

    class GUIManip_Calibrator : public GUIManipulator {
        FXDECLARE(GUIManip_Calibrator)
    public:
        enum {
            MID_USER_DEF = FXDialogBox::ID_LAST,
            MID_PRE_DEF,
            MID_OPTION,
            MID_CLOSE,
            ID_LAST
        };
        /// Constructor
        GUIManip_Calibrator(GUIMainWindow& app,
                            const std::string& name, GUICalibrator& o,
                            int xpos, int ypos);

        /// Destructor
        virtual ~GUIManip_Calibrator();

        long onCmdOverride(FXObject*, FXSelector, void*);
        long onCmdClose(FXObject*, FXSelector, void*);
        long onCmdUserDef(FXObject*, FXSelector, void*);
        long onUpdUserDef(FXObject*, FXSelector, void*);
        long onCmdPreDef(FXObject*, FXSelector, void*);
        long onUpdPreDef(FXObject*, FXSelector, void*);
        long onCmdChangeOption(FXObject*, FXSelector, void*);

    private:
        GUIMainWindow* myParent;

        FXint myChosenValue;

        FXDataTarget myChosenTarget;

        double mySpeed;

        FXDataTarget mySpeedTarget;

        FXRealSpinner* myUserDefinedSpeed;

        FXComboBox* myPredefinedValues;

        GUICalibrator* myObject;

    protected:
        GUIManip_Calibrator() { }

    };

private:
    /// Definition of a positions container
    typedef std::vector<Position> PosCont;

    /// Definition of a rotation container
    typedef std::vector<double> RotCont;

    /// @brief the calibrator being wrapped
    MSCalibrator* myCalibrator;

    /// The positions in full-geometry mode
    PosCont myFGPositions;

    /// The rotations in full-geometry mode
    RotCont myFGRotations;

    /// The boundary of this rerouter
    Boundary myBoundary;

    /// The information whether the speed shall be shown in m/s or km/h
    bool myShowAsKMH;

};
