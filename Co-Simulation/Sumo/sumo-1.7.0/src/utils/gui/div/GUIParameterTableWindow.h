/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUIParameterTableWindow.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// The window that holds the table of an object's parameter
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <fx.h>
#include <utils/common/ValueSource.h>
#include <utils/common/SUMOTime.h>
#include "GUIParameterTableItem.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GUIGlObject;
class GUIMainWindow;
class GUIParameterTableItemInterface;
class Parameterised;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIParameterTableWindow
 * @brief A window containing a gl-object's parameter
 *
 * This class realises a view on some parameter of a gl-object. The gl-object
 *  itself is responsible for filling this table.
 *
 * After allocating such a table, the object should fill the rows by calling
 *  one of the mkItem-methods for each. The building is closed using closeBuilding
 *  what forces the table to be displayed.
 *
 * Each row is represented using an instance of GUIParameterTableItemInterface.
 */
class GUIParameterTableWindow : public FXMainWindow {
    FXDECLARE(GUIParameterTableWindow)
public:
    /** @brief Constructor
     *
     * @param[in] app The application this window belongs to
     * @param[in] o The gl-object this table describes
     */
    GUIParameterTableWindow(GUIMainWindow& app, GUIGlObject& o);


    /// @brief Destructor
    ~GUIParameterTableWindow();


    /** @brief Closes the building of the table
     *
     * Adds the table to the list of child windows of the applications also
     *  telling the application that the window should be updated in each
     *  simulation step. Shows the table.
     *
     * @see GUIMainWindow::addChild
     */
    void closeBuilding(const Parameterised* p = 0);



    /** @brief Lets this window know the object shown is being deleted
     * @param[in] o The deleted (shown) object
     */
    void removeObject(GUIGlObject* const o);



    /// @name Row adding functions
    /// @{

    /** @brief Adds a row which obtains its value from a ValueSource
     *
     * @param[in] name The name of the row entry
     * @param[in] dynamic Information whether the entry is dynamic
     * @param[in] src The value source to use
     */
    template<class T>
    void mkItem(const char* name, bool dynamic, ValueSource<T>* src) {
        myTable->insertRows((int)myItems.size() + 1);
        GUIParameterTableItemInterface* i = new GUIParameterTableItem<T>(myTable, myCurrentPos++, name, dynamic, src);
        myItems.push_back(i);
    }

    /** @brief Adds a row which shows a string-value
     *
     * @param[in] name The name of the row entry
     * @param[in] dynamic Information whether the entry is dynamic
     * @param[in] value The value to show
     * @todo the dynamic-parameter is obsolete(?)
     */
    void mkItem(const char* name, bool dynamic, std::string value);


    /** @brief Adds a row which shows a unsigned-value
     *
     * @param[in] name The name of the row entry
     * @param[in] dynamic Information whether the entry is dynamic
     * @param[in] value The value to show
     * @todo the dynamic-parameter is obsolete
     */
    void mkItem(const char* name, bool dynamic, unsigned value);


    /** @brief Adds a row which shows a integer-value
     *
     * @param[in] name The name of the row entry
     * @param[in] dynamic Information whether the entry is dynamic
     * @param[in] value The value to show
     * @todo the dynamic-parameter is obsolete
     */
    void mkItem(const char* name, bool dynamic, int value);


    /** @brief Adds a row which shows a 64 bit integer-value
     *
     * @param[in] name The name of the row entry
     * @param[in] dynamic Information whether the entry is dynamic
     * @param[in] value The value to show
     * @todo the dynamic-parameter is obsolete
     */
    void mkItem(const char* name, bool dynamic, long long int value);


    /** @brief Adds a row which shows a double-value
     *
     * @param[in] name The name of the row entry
     * @param[in] dynamic Information whether the entry is dynamic
     * @param[in] value The value to show
     * @todo the dynamic-parameter is obsolete
     */
    void mkItem(const char* name, bool dynamic, double value);


    /// @}



    /// @name FOX-callbacks
    /// @{

    /** @brief Updates the table due to a simulation step */
    long onSimStep(FXObject*, FXSelector, void*);

    /** @brief Does nothing
     * @todo Recheck whether this is needed (to override FXTable-behaviour?)
     */
    long onTableSelected(FXObject*, FXSelector, void*);

    /** @brief Does nothing
     * @todo Recheck whether this is needed (to override FXTable-behaviour?)
     */
    long onTableDeselected(FXObject*, FXSelector, void*);


    /** @brief Shows a popup
     *
     * Callback for right-mouse-button pressing event. Obtains the selected row
     *  and determines whether it is dynamic. If so, a popup-menu which allows
     *  to open a tracker for this value is built and shown.
     *
     * @see GUIParameterTableItemInterface
     * @see GUIParam_PopupMenuInterface
     */
    long onRightButtonPress(FXObject*, FXSelector, void*);

    /// @brief directly opens tracker when clicking on last column
    long onLeftBtnPress(FXObject*, FXSelector, void*);
    /// @}

    /** @brief Updates all instances
     */
    static void updateAll() {
        FXMutexLock locker(myGlobalContainerLock);
        std::for_each(myContainer.begin(), myContainer.end(), std::mem_fun(&GUIParameterTableWindow::updateTable));
    }

protected:
    /** @brief Updates the table
     *
     * Goes through all entries and updates them using GUIParameterTableItemInterface::update.
     *
     * @see GUIParameterTableItemInterface::update
     */
    void updateTable();

    /// @brief The mutex used to avoid concurrent updates of the instance container
    static FXMutex myGlobalContainerLock;

    /// @brief The container of items that shall be updated
    static std::vector<GUIParameterTableWindow*> myContainer;

private:
    /// @brief The object to get the information from
    GUIGlObject* myObject;

    /// @brief The table to display the information in
    FXTable* myTable;

    /// @brief The main application window
    GUIMainWindow* myApplication;

    /// @brief The list of table rows
    std::vector<GUIParameterTableItemInterface*> myItems;

    /// @brief y-position for opening new tracker window
    int myTrackerY;

    /// @brief The index of the next row to add - used while building
    unsigned myCurrentPos;

    /// @brief A lock assuring save updates in case of object deletion
    mutable FXMutex myLock;

    /// @brief returns the number of parameters if obj is Parameterised and 0 otherwise
    static int numParams(const GUIGlObject* obj);

protected:
    FOX_CONSTRUCTOR(GUIParameterTableWindow)

};
