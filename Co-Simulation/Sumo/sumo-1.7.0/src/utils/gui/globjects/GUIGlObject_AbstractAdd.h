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
/// @file    GUIGlObject_AbstractAdd.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    2004
///
// Base class for additional objects (detectors etc.)
/****************************************************************************/
#pragma once
#include <config.h>

#include "GUIGlObjectTypes.h"
#include "GUIGlObject.h"
#include <vector>
#include <map>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIGlObject_AbstractAdd
 */
class GUIGlObject_AbstractAdd : public GUIGlObject {

public:
    /// @brief constructor
    GUIGlObject_AbstractAdd(GUIGlObjectType type, const std::string& id);

    /// @brief destructor
    ~GUIGlObject_AbstractAdd();

    /// @brief Clears the dictionary (the objects will not be deleted)
    static void clearDictionary();

    /// @brief Returns a named object
    static GUIGlObject_AbstractAdd* get(const std::string& name);

    /// @brief Removes an object
    static void remove(GUIGlObject_AbstractAdd* o);

    /// @brief Returns the list of all additional objects
    static const std::vector<GUIGlObject_AbstractAdd*>& getObjectList();

    /// @brief Returns the list of gl-ids of all additional objects that match the given type
    static std::vector<GUIGlID> getIDList(GUIGlObjectType typeFilter);

protected:
    /// @brief Map from names of loaded additional objects to the objects themselves
    static std::map<std::string, GUIGlObject_AbstractAdd*> myObjects;

    /// @brief The list of all addtional objects currently loaded
    static std::vector<GUIGlObject_AbstractAdd*> myObjectList;
};
