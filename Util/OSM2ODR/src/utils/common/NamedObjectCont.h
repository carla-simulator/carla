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
/// @file    NamedObjectCont.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// A map of named object pointers
/****************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include <algorithm>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NamedObjectCont
 * @brief A map of named object pointers
 *
 * An associative storage (map) for objects (pointers to them to be exact),
 *  which do have a name.
 */
template<class T>
class NamedObjectCont {
public:
    /// @brief Definition of the key to pointer map type
    typedef std::map< std::string, T > IDMap;

    ///@brief Destructor
    virtual ~NamedObjectCont() {
        // iterate over all elements to delete it
        for (auto i : myMap) {
            delete i.second;
        }
    }

    /** @brief Adds an item
     *
     * If another item with the same name is already known, false is reported
     *  and the item is not added.
     *
     * @param[in] id The id of the item to add
     * @param[in] item The item to add
     * @return If the item could been added (no item with the same id was within the container before)
     */
    bool add(const std::string& id, T item) {
        if (myMap.find(id) != myMap.end()) {
            return false;
        }
        myMap.insert(std::make_pair(id, item));
        return true;
    }

    /** @brief Removes an item
     * @param[in] id The id of the item to remove
     * @param[in] del delete item after removing of container
     * @return If the item could been removed (an item with the id was within the container before)
     */
    bool remove(const std::string& id, const bool del = true) {
        auto it = myMap.find(id);
        if (it == myMap.end()) {
            return false;
        } else {
            if (del) {
                delete it->second;
            }
            myMap.erase(it);
            return true;
        }
    }

    /** @brief Retrieves an item
     *
     * Returns 0 when no item with the given id is stored within the container
     *
     * @param[in] id The id of the item to retrieve
     * @return The item stored under the given id, or 0 if no such item exists
     */
    T get(const std::string& id) const {
        auto it = myMap.find(id);
        if (it == myMap.end()) {
            return 0;
        } else {
            return it->second;
        }
    }

    /// @brief Removes all items from the container (deletes them, too)
    void clear() {
        for (auto i : myMap) {
            delete i.second;
        }
        myMap.clear();
    }

    /// @brief Returns the number of stored items within the container
    int size() const {
        return (int) myMap.size();
    }

    /* @brief Fills the given vector with the stored objects' ids
     * @param[in] into The container to fill
     */
    void insertIDs(std::vector<std::string>& into) const {
        for (auto i : myMap) {
            into.push_back(i.first);
        }
    }

    /// @brief change ID of a stored object
    bool changeID(const std::string& oldId, const std::string& newId) {
        auto i = myMap.find(oldId);
        if (i == myMap.end()) {
            return false;
        } else {
            // save Item, remove it from Map, and insert it again with the new ID
            T item = i->second;
            myMap.erase(i);
            myMap.insert(std::make_pair(newId, item));
            return true;
        }
    }

    /// @brief Returns a reference to the begin iterator for the internal map
    typename IDMap::const_iterator begin() const {
        return myMap.begin();
    }

    /// @brief Returns a reference to the end iterator for the internal map
    typename IDMap::const_iterator end() const {
        return myMap.end();
    }


private:
    /// @brief The map from key to object
    IDMap myMap;
};
