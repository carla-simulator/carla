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
/// @file    GUISelectedStorage.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Jun 2004
///
// Storage for "selected" objects
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <string>
#include <map>
#include <fstream>
#include <fx.h>
#include <utils/common/UtilExceptions.h>
#include <utils/gui/globjects/GUIGlObject.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUISelectedStorage
 * @brief Storage for "selected" objects
 *
 * Object selection is done by storing the "gl-ids" of selectable objects
 *  (GUIGlObjects) within internal containers of this class. Each id is stored
 *  twice - in a global container and within one of the type-aware containers.
 *
 * This class allows adding and removing objects (their ids) from the lists
 *  of selected objects, saving the lists into a file, and obtaining the lists
 *  of selected objects (both all and type-aware).
 *
 * Most of the adding/removing methods do not require a GUIGlObjectType as
 *  parameter, but an integer. This is done to perform the action on objects with
 *  a yet unknown type - in this case, the type is obtained internally.
 *
 * Besides this, the class forces an active view of selected items
 *  to refresh its contents if an item is added/removed. For this, an
 *  FXWindow has to make itself visible to GUISelectedStorage.
 *
 * @see GUIGlObject
 * @see GUIGlObjectType
 * @see GUIDialog_GLChosenEditor
 */
class GUISelectedStorage {

public:
    /// @class update target
    class UpdateTarget {

    public:
        /// @brief virtual destructor
        virtual ~UpdateTarget() {};

        /// @brief called when selection is updated
        virtual void selectionUpdated() = 0;
    };

public:
    /// @brief Constructor
    GUISelectedStorage();

    /// @brief Destructor
    ~GUISelectedStorage();

    /** @brief Returns the information whether the object with the given type and id is selected
     *
     * If the type is ==-1, it is determined, first. If it could not be obtained,
     *  or if the type is not covered by any selection container, a ProcessError is thrown.
     *
     * Otherwise, the container holding objects of the determined type is
     *  asked whether the given id is stored using SingleTypeSelections::isSelected().
     *
     * @param[in] type The type of the object (GUIGlObjectType or -1)
     * @param[in] id The id of the object
     * @return Whether the object is selected
     * @exception ProcessError If the object is not known or the type is not covered by a sub-container
     * @see GUIGlObject
     * @see GUIGlObjectType
     * @see SingleTypeSelections::isSelected
     */
    bool isSelected(GUIGlObjectType type, GUIGlID id);

    bool isSelected(const GUIGlObject* o);

    /** @brief Adds the object with the given id
     *
     * The id of the object is added to the sub-container that is
     *  responsible for objects of the determined type using SingleTypeSelections::select
     *  and to the global list of chosen items if it is not already there.
     *
     * The optionally listening window is informed about the change.
     *
     * @param[in] id The id of the object
     * @exception ProcessError If the object is not known or the type is not covered by a sub-container
     * @see GUIGlObject
     * @see GUIGlObjectType
     * @see SingleTypeSelections::select
     * @see GUIDialog_GLChosenEditor
     */
    void select(GUIGlID id, bool update = true);

    /** @brief Deselects the object with the given id
     *
     * The id of the object is removed from the sub-container that is
     *  responsible for objects of the determined type using SingleTypeSelections::deselect
     *  and from the global list of chosen items if it is there.
     *
     * The optionally listening UpdateTarget is informed about the change.
     *
     * @param[in] id The id of the object
     * @exception ProcessError If the object is not known or the type is not covered by a sub-container
     * @see GUIGlObject
     * @see GUIGlObjectType
     * @see SingleTypeSelections::deselect
     * @see GUIDialog_GLChosenEditor
     */
    void deselect(GUIGlID id);

    /** @brief Toggles selection of an object
     *
     * If the object can not be obtained a ProcessError is thrown.
     *
     * Otherwise, it is determined whether the object is already selected or not.
     *  If so, it is deselected by calling "deselect", otherwise it is selected
     *  via "select".
     *
     * @param[in] id The id of the object
     * @exception ProcessError If the object is not known or the type is not covered by a sub-container
     * @see GUIGlObject
     * @see deselect
     * @see select
     */
    void toggleSelection(GUIGlID id);

    /// @brief Returns the set of ids of all selected objects
    const std::set<GUIGlID>& getSelected() const;

    /**  @brief Returns the set of ids of all selected objects' of a certain type
     *
     * @param[in] type The type of the object
     * @return A set containing the ids of all selected objects of the given type
     * @see SingleTypeSelections::getSelected
     */
    const std::set<GUIGlID>& getSelected(GUIGlObjectType type);

    /** @brief Clears the list of selected objects
     *
     * Clears the global container and all sub-containers via SingleTypeSelections::clear.
     *
     * The optionally listening UpdateTarget is informed about the change.
     */
    void clear();

    /** @brief Loads a selection list (optionally with restricted type)
     *
     * @param[in] filename The name of the file to load the list of selected objects from
     * @param[in] type The type of the objects to load if changed from default
     * @return error messages if errors occurred or the empty string
     */
    std::string load(const std::string& filename, GUIGlObjectType type = GLO_MAX);

    /** @brief Loads a selection list (optionally with restricted type) and
     * returns the ids of all active objects
     *
     * @param[in] filename The name of the file to load the list of selected objects from
     * @param[out] msg Any error messages while loading or the empty string
     * @param[in] type The type of the objects to load if changed from default
     * @param[in] maxErrors The maximum Number of errors to return
     * @return the set of loaded ids
     */
    std::set<GUIGlID> loadIDs(const std::string& filename, std::string& msgOut, GUIGlObjectType type = GLO_MAX, int maxErrors = 16);

    /** @brief Saves a selection list
     *
     * @param[in] type The type of the objects to save
     * @param[in] filename The name of the file to save the list of selected objects into
     */
    void save(GUIGlObjectType type, const std::string& filename);

    /** @brief Saves the combined selection of all types
     *
     * @param[in] filename The name of the file to save the list of selected objects into
     */
    void save(const std::string& filename) const;

    /** @brief Adds a dialog to be updated
     * @param[in] updateTarget the callback for selection changes
     */
    void add2Update(UpdateTarget* updateTarget);

    /// @brief @brief Removes the dialog to be updated
    void remove2Update();

    /**
     * @class SingleTypeSelections
     * @brief A container for ids of selected objects of a certain type.
     */
    class SingleTypeSelections {

    public:
        /// @brief Constructor
        SingleTypeSelections();

        /// @brief Destructor
        ~SingleTypeSelections();

        /** @brief Returns the information whether the object with the given id is qithin the selection
         * @param[in] id The id of the object
         * @return Whether the object is selected
         */
        bool isSelected(GUIGlID id);

        /** @brief Adds the object with the given id to the list of selected objects
         * @param[in] id The id of the object
         */
        void select(GUIGlID id);

        /** @brief Deselects the object with the given id from the list of selected objects
         * @param[in] id The id of the object
         */
        void deselect(GUIGlID id);

        /// @brief Clears the list of selected objects
        void clear();

        /** @brief Saves the list of selected objects to a file named as given
         * @param[in] filename The name of the file to save the list into
         */
        void save(const std::string& filename);

        /** @brief Returns the list of selected ids
         * @return A list containing the ids of all selected objects
         */
        const std::set<GUIGlID>& getSelected() const;

    private:
        /// @brief The list of selected ids
        std::set<GUIGlID> mySelected;

    };

    /// @brief set SingleTypeSelections as friend class
    friend class SingleTypeSelections;


private:
    /// @brief map with the selections
    std::map<GUIGlObjectType, SingleTypeSelections> mySelections;

    /// @brief List of selected objects
    std::set<GUIGlID> myAllSelected;

    /// @brief The dialog to be updated
    UpdateTarget* myUpdateTarget;

    /// @brief load items into the given set, optionally restricting to type
    std::string load(GUIGlObjectType type, const std::string& filename, bool restrictType, std::set<GUIGlID>& into);

    /// @brief saves items from the given set
    static void save(const std::string& filename, const std::set<GUIGlID>& ids);
};
