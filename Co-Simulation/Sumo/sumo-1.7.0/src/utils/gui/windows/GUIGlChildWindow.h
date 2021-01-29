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
/// @file    GUIGlChildWindow.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Fri, 29.04.2005
///
//
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>

#include "GUISUMOAbstractView.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class GUIGlChildWindow : public FXMDIChild {
    FXDECLARE(GUIGlChildWindow)
public:
    /// @brief constructor
    GUIGlChildWindow(FXMDIClient* p, GUIMainWindow* parentWindow,
                     FXMDIMenu* mdimenu, const FXString& name, FXMenuBar* gripNavigationToolbar,
                     FXIcon* ic = NULL, FXuint opts = 0, FXint x = 0, FXint y = 0, FXint w = 0, FXint h = 0);

    /// @brief destructor
    virtual ~GUIGlChildWindow();

    /// @brief get build GL Canvas
    virtual FXGLCanvas* getBuildGLCanvas() const;

    /// @brief create GUIGlChildWindow
    virtual void create();

    /// @brief return GUISUMOAbstractView
    GUISUMOAbstractView* getView() const;

    /** @brief Returns the main window
     * @return This view's parent
     */
    GUIMainWindow* getParent();

    /// @name buttons call backs
    /// @{
    long onCmdRecenterView(FXObject*, FXSelector, void*);
    long onCmdEditViewport(FXObject*, FXSelector, void*);
    long onCmdEditViewScheme(FXObject*, FXSelector, void*);
    long onCmdShowToolTips(FXObject* sender, FXSelector, void*);
    long onCmdZoomStyle(FXObject* sender, FXSelector, void*);
    long onCmdChangeColorScheme(FXObject*, FXSelector sel, void*);
    /// @}

    /// @brief return a reference to navigation toolbar
    FXToolBar* getNavigationToolBar(GUISUMOAbstractView& v);

    /// @ brief return a pointer to locator popup
    FXPopup* getLocatorPopup();

    /// @brief return combobox with the current coloring schemes (standard, fastest standard, real world...)
    FXComboBox* getColoringSchemesCombo();

    /** @brief Centers the view onto the given artifact
     * @param[in] id The id of the object to center the view on
     */
    void setView(GUIGlID id);

    /// @brief true if the object is selected (may include extra logic besides calling gSelected)
    virtual bool isSelected(GUIGlObject* o) const;

protected:
    FOX_CONSTRUCTOR(GUIGlChildWindow)

    /// @brief The parent window
    GUIMainWindow* myParent;

    /// @brief The grip navigation tool bar
    FXMenuBar* myGripNavigationToolbar;

    /// @brief The static navigation tool bar
    FXToolBar* myStaticNavigationToolBar;

    /// @brief The view
    GUISUMOAbstractView* myView;

    /// The locator menu
    FXPopup* myLocatorPopup;

    /// @brief The locator button
    FXMenuButton* myLocatorButton;

    /// @brief The contents frame
    FXVerticalFrame* myContentFrame;

    /// @brief The coloring schemes
    FXComboBox* myColoringSchemes;

    /// @brief build navigation toolbar
    void buildNavigationToolBar();

    /// @brief build coloring toolbar
    void buildColoringToolBar();

    /// @brief build screenshot toolbar
    void buildScreenshotToolBar();

private:
    /// @brief Invalidated copy constructor.
    GUIGlChildWindow(const GUIGlChildWindow&) = delete;

    /// @brief Invalidated assignment operator.
    GUIGlChildWindow& operator=(const GUIGlChildWindow&) = delete;
};
