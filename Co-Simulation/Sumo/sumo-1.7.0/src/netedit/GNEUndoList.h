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
/// @file    GNEUndoList.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// FXUndoList is pretty dandy but some features are missing:
//   - we cannot find out wether we have currently begun an undo-group and
//     thus abort() is hard to use.
//   - onUpd-methods do not disable undo/redo while in an undo-group
//
// GNEUndoList inherits from FXUndoList and patches some methods. these are
// prefixed with p_
/****************************************************************************/
#pragma once
#include <config.h>

#include <stack>
#include <string>
#include <fx.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GNEChange_Attribute;
class GNEApplicationWindow;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class  GNEUndoList
 */
class GNEUndoList : public FXUndoList {
    /// @brief FOX declaration
    FXDECLARE_ABSTRACT(GNEUndoList)

public:
    /// @brief constructor
    /// @note be aware that "parent" may be not fully initialized when stored here, so don't call any methods on it.
    GNEUndoList(GNEApplicationWindow* parent);

    /**@brief Begin undo command sub-group. This begins a new group of commands that
     * are treated as a single command.  Must eventually be followed by a
     * matching end() after recording the sub-commands. The new sub-group
     * will be appended to its parent group's undo list when end() is called.
     */
    void p_begin(const std::string& description);

    /**@brief End undo command sub-group.  If the sub-group is still empty, it will
     * be deleted; otherwise, the sub-group will be added as a new command
     * into parent group.
     * A matching begin() must have been called previously.
     */
    void p_end();

    /// @brief clears the undo list (implies abort)
    void p_clear();

    /// @brief reverts and discards ALL active command groups
    void p_abort();

    /// @brief reverts last command group
    void p_abortLastCommandGroup();

    /// @brief undo the last command group
    void undo();

    /// @brief redo the last command group
    void redo();

    /// @brief special method, avoid empty changes, always execute
    void p_add(GNEChange_Attribute* cmd);

    /// @brief get size of current CommandGroup
    int currentCommandGroupSize() const;

    /// @name FOX-callbacks
    /// @{
    /// @brief event after Undo
    long p_onUpdUndo(FXObject*, FXSelector, void*);

    /// @brief event after Redo
    long p_onUpdRedo(FXObject*, FXSelector, void*);
    /// @}

    /// @brief Check if undoList has command group
    bool hasCommandGroup() const;

private:
    /// @brief class CommandGroup
    class CommandGroup : public FXCommandGroup {
    public:
        /// @brief Constructor
        CommandGroup(std::string description);

        /// @brief get description
        const std::string& getDescription();

        /// @brief get undo Name
        FXString undoName() const;

        /// @brief get redo name
        FXString redoName() const;

    private:
        /// @brief description of command
        const std::string myDescription;
    };

    // @brief the stack of currently active command groups
    std::stack<CommandGroup*> myCommandGroups;

    // @brief the parent GNEApplicationWindow for this undolist
    GNEApplicationWindow* const myGNEApplicationWindowParent;
};
