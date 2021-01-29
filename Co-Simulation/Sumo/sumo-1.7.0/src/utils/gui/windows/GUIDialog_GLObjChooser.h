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
/// @file    GUIDialog_GLObjChooser.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Class for the window that allows to choose a street, junction or vehicle
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <set>
#include <fx.h>
#include <utils/gui/globjects/GUIGlObject.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUIGlChildWindow;
class GUIGlObjectStorage;
class GUIGlObject;


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class GUIDialog_GLObjChooser
 * Instances of this class are windows that display the list of instances
 * from a given artifact like vehicles, edges or junctions and allow
 * one of their items
 */
class GUIDialog_GLObjChooser : public FXMainWindow {
    // FOX-declarations
    FXDECLARE(GUIDialog_GLObjChooser)

public:
    /** @brief Constructor
     * @param[in] parent The calling view
     * @param[in] icon The icon to use
     * @param[in] title The title to use
     * @param[in] glStorage The storage to retrieve ids from
     */
    GUIDialog_GLObjChooser(GUIGlChildWindow* parent, FXIcon* icon, const FXString& title,
                           const std::vector<GUIGlID>& ids,
                           GUIGlObjectStorage& glStorage);


    /// @brief Destructor
    virtual ~GUIDialog_GLObjChooser();


    /** @brief Returns the chosen (selected) object
     * @return The selected object
     */
    GUIGlObject* getObject() const {
        return static_cast<GUIGlObject*>(mySelected);
    }



    /// @name FOX-callbacks
    /// @{

    /// @brief Callback: The selected item shall be centered within the calling view
    long onCmdCenter(FXObject*, FXSelector, void*);

    /// @brief Callback: The selected vehicle shall be tracked within the calling view
    long onCmdTrack(FXObject*, FXSelector, void*);

    /// @brief Callback: The dialog shall be closed
    long onCmdClose(FXObject*, FXSelector, void*);

    /// @brief Callback: Something has been typed into the the field
    long onChgText(FXObject*, FXSelector, void*);

    /// @brief Callback: Selects to current item if enter is pressed
    long onCmdText(FXObject*, FXSelector, void*);

    /// @brief Callback: Selects to current item if enter is pressed
    long onListKeyPress(FXObject*, FXSelector, void*);

    /// @brief Callback: Hides unselected items if pressed
    long onCmdFilter(FXObject*, FXSelector, void*);

    /// @brief Callback: Hides unmatched items if pressed
    long onCmdFilterSubstr(FXObject*, FXSelector, void*);

    /// @brief Callback: Toggle selection status of current object
    long onCmdToggleSelection(FXObject*, FXSelector, void*);

    /// @brief Callback: Toggle locator by name
    long onCmdLocateByName(FXObject*, FXSelector, void*);
    /// @}

    /// @brief sets the focus after the window is created to work-around bug in libfox
    void show();

protected:

    /// @brief toggle selection (handled differently in NETEDIT)
    virtual void toggleSelection(int listIndex);

    /// update the list with the given ids
    void refreshList(const std::vector<GUIGlID>& ids);

    /// @bbrief retrieve name for the given object
    virtual std::string getObjectName(GUIGlObject* o) const;

private:
    /// @brief The list that holds the ids
    FXList* myList;

    /// @brief The button that triggers centering on the select object
    FXButton* myCenterButton;

    /// @brief The button that triggers tracking on the select vehicle
    FXButton* myTrackButton;

    /// @brief The parent window
    GUIGlChildWindow* myParent;

    /// @brief The chosen id
    GUIGlObject* mySelected;

    /// @brief The text field
    FXTextField* myTextEntry;

    /// myList contains (void) pointers to elements of myIDs instead of the more
    //volatile pointers to GUIGlObject
    std::set<GUIGlID> myIDs;

    /// @brief whether to locate by object name instead of id
    bool myLocateByName;

    /// @brief whether the list was filter by substring
    bool myHaveFilteredSubstring;

protected:
    FOX_CONSTRUCTOR(GUIDialog_GLObjChooser)

};
