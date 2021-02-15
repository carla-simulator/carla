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
/// @file    GUIDialog_EditViewport.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2005-05-04
///
// A dialog to change the viewport
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>

// ===========================================================================
// class declarations
// ===========================================================================
class GUISUMOAbstractView;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIDialog_EditViewport
 * @brief A dialog to change the viewport
 */
class GUIDialog_EditViewport : public FXDialogBox {
    // FOX-declarations
    FXDECLARE(GUIDialog_EditViewport)
public:
    /// @brief FOX-callback enumerations
    enum {
        MID_CHANGED = FXDialogBox::ID_LAST,
        MID_OK,
        MID_CANCEL,
        MID_LOAD,
        MID_SAVE
    };


    /** @brief Constructor
     * @param[in] parent The view to change
     * @param[in] name This dialog's caption
     * @param[in] x x-position to show the dialog at
     * @param[in] y y-position to show the dialog at
     */
    GUIDialog_EditViewport(GUISUMOAbstractView* parent,  const char* name, int x, int y);

    /// @brief Destructor
    ~GUIDialog_EditViewport();

    /// @brief overload show function to focus always in OK Button
    void show();

    /// @name FOX-callbacks
    /// @{

    /// Called when the user changes the viewport
    long onCmdChanged(FXObject*, FXSelector, void*);

    /// Called when the user wants to keep the viewport
    long onCmdOk(FXObject*, FXSelector, void*);

    /// Called when the user wants to restore the viewport
    long onCmdCancel(FXObject*, FXSelector, void*);

    /// Called when the user wants to load a viewport
    long onCmdLoad(FXObject*, FXSelector, void*);

    /// Called when the user wants to save a viewport
    long onCmdSave(FXObject*, FXSelector, void*);
    /// @}

    /// write the settings to the given device
    void writeXML(OutputDevice& dev);

    /** @brief Sets the given values into the dialog
     * @param[in] zoom Current view's zoom
     * @param[in] xoff Current view's x-offset
     * @param[in] yoff Current view's y-offset
     */
    void setValues(double zoom, double xoff, double yoff, double rotation);

    /** @brief Sets the given values into the dialog
     * @param[in] lookFrom Current viewport's from
     * @param[in] lookAt Current viewport's at
     */
    void setValues(const Position& lookFrom, const Position& lookAt, double rotation);

    /** @brief Resets old values
     * @param[in] lookFrom Current viewport's from
     * @param[in] lookAt Current viewport's at
     */
    void setOldValues(const Position& lookFrom, const Position& lookAt, double rotation);

    /** @brief Returns the information whether one of the spin dialers is grabbed
    * @return Whether the spin dialers are currently used
    */
    bool haveGrabbed() const;

protected:
    FOX_CONSTRUCTOR(GUIDialog_EditViewport)

    /// @brief save window position to the registry
    void saveWindowPos();

private:
    /// @brief The calling view
    GUISUMOAbstractView* myParent;

    /// @brief The old viewport
    Position myOldLookFrom, myOldLookAt;
    double myOldRotation;

    /// @brief load button
    FXButton* myLoadButton;

    /// @brief save button
    FXButton* mySaveButton;

    /// @brief The spin dialers used to change the view
    FXRealSpinner* myZoom, *myXOff, *myYOff, *myZOff, *myRotation;

    /// @brief The spin dialers used to change the view at (osg only)
    FXRealSpinner* myLookAtX, *myLookAtY, *myLookAtZ;

    /// @brief OK button
    FXButton* myOKButton;

    /// @brief Cancel button
    FXButton* myCancelButton;
};
