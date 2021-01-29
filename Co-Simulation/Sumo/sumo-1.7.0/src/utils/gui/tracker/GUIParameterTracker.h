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
/// @file    GUIParameterTracker.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A window which displays the time line of one (or more) value(s)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <fx.h>
// fx3d includes windows.h so we need to guard against macro pollution
#ifdef WIN32
#define NOMINMAX
#endif
#include <fx3d.h>
#ifdef WIN32
#undef NOMINMAX
#endif
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/div/GLObjectValuePassConnector.h>
#include "TrackerValueDesc.h"


// ===========================================================================
// class definitions
// ===========================================================================
/** @class GUIParameterTracker
 * @brief A window which displays the time line of one (or more) value(s)
 */
class GUIParameterTracker : public FXMainWindow {
    FXDECLARE(GUIParameterTracker)
public:
    /// @brief callback-enumerations
    enum {
        /// @brief Change aggregation interval
        MID_AGGREGATIONINTERVAL = FXMainWindow::ID_LAST,
        /// @brief Save the current values
        MID_SAVE,
        /// @brief end-of-enum
        ID_LAST
    };


    /** @brief Constructor (the tracker is empty)
     * @param[in] app The main application window
     * @param[in] name The title of the tracker
     */
    GUIParameterTracker(GUIMainWindow& app, const std::string& name);


    /// @brief Destructor
    ~GUIParameterTracker();


    /// @brief Creates the window
    void create();


    /** @brief Adds a further time line to display
     * @param[in] o The object to get values from
     * @param[in] src The value source of the object
     * @param[in] newTracked The description of the tracked value
     */
    void addTracked(GUIGlObject& o, ValueSource<double>* src,
                    TrackerValueDesc* newTracked);


    /// @name FOX-callbacks
    /// @{

    /// @brief Called on window resizing
    long onConfigure(FXObject*, FXSelector, void*);

    /// @brief Called if the window shall be repainted
    long onPaint(FXObject*, FXSelector, void*);

    /// @brief Called on a simulation step
    long onSimStep(FXObject*, FXSelector, void*);

    /// @brief Called when the aggregation interval (combo) has been changed
    long onCmdChangeAggregation(FXObject*, FXSelector, void*);

    /// @brief Called when the data shall be saved
    long onCmdSave(FXObject*, FXSelector, void*);
    /// @}


public:
    /**
     * @class GUIParameterTrackerPanel
     * This panel lies within the GUIParameterTracker being the main widget.
     * It is the widget responsible for displaying the information while
     * GUIParameterTracker only provides window-facilities.
     */
    class GUIParameterTrackerPanel : public FXGLCanvas {
        FXDECLARE(GUIParameterTrackerPanel)
    public:
        /** @brief Constructor
         * @param[in] c The parent composite
         * @param[in] app The main window
         * @param[in] parent The parent tracker window this view belongs to
         */
        GUIParameterTrackerPanel(FXComposite* c, GUIMainWindow& app,
                                 GUIParameterTracker& parent);

        /// @brief Destructor
        ~GUIParameterTrackerPanel();

        /// @brief needed to update
        friend class GUIParameterTracker;


        /// @name FOX-callbacks
        /// @{

        /// Called on window resizing
        long onConfigure(FXObject*, FXSelector, void*);

        /// Called if the window shall be repainted
        long onPaint(FXObject*, FXSelector, void*);

        /// Called on a simulation step
        long onSimStep(FXObject* sender, FXSelector, void*);
        /// @}


    private:
        /** @brief Draws all values
         */
        void drawValues();

        /** @brief Draws a single value
         * @param[in] desc The tracked values to draw
         * @param[in] namePos Position to display the name at (currently unused)
         */
        void drawValue(TrackerValueDesc& desc, double namePos);


    private:
        /// @brief The parent window
        GUIParameterTracker* myParent;

        /// @brief the sizes of the window
        int myWidthInPixels, myHeightInPixels;

        /// @brief The main application
        GUIMainWindow* myApplication;

    protected:
        FOX_CONSTRUCTOR(GUIParameterTrackerPanel)
    };

public:
    /// @brief the panel may change some things
    friend class GUIParameterTrackerPanel;

private:
    /// @brief Builds the tool bar
    void buildToolBar();


protected:
    /// @brief The main application
    GUIMainWindow* myApplication;

    /// @brief The list of tracked values
    std::vector<TrackerValueDesc*> myTracked;

    /// @brief The panel to display the values in
    GUIParameterTrackerPanel* myPanel;

    /// @brief The value sources
    std::vector<GLObjectValuePassConnector<double>*> myValuePassers;

    /// @brief for some menu detaching fun
    FXToolBarShell* myToolBarDrag;

    /// @brief A combo box to select an aggregation interval
    FXComboBox* myAggregationInterval;

    /// @brief The simulation delay
    FXdouble myAggregationDelay;

    /// @brief The tracker tool bar
    FXToolBar* myToolBar;

protected:
    FOX_CONSTRUCTOR(GUIParameterTracker)

};
