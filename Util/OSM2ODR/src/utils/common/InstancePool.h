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
/// @file    InstancePool.h
/// @author  Daniel Krajzewicz
/// @date    Fri, 29.04.2005
///
// A pool of resuable instances
/****************************************************************************/
#pragma once
#include <vector>
#include <algorithm>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class InstancePool
 * @brief A pool of resuable instances
 */
template<typename T>
class InstancePool {
public:
    /** @brief Constructor
     *
     * @param[in] deleteOnQuit Information whether stored instances shall be deleted when this container is deleted
     */
    InstancePool(bool deleteOnQuit) : myDeleteOnQuit(deleteOnQuit) { }


    /// @brief Destructor
    ~InstancePool() {
        typedef typename std::vector<T*>::iterator It;
        if (myDeleteOnQuit) {
            for (It i = myFreeInstances.begin(); i != myFreeInstances.end(); i++) {
                delete *i;
            }
        }
    }


    /** @brief Returns a free instance or 0 if no such exists
     *
     * If any free instance is stored, it is returned and removed from
     *  the storage. If no one is stored, 0 is returned.
     *
     * @return A free instance or 0 if no such exists
     */
    T* getFreeInstance() {
        if (myFreeInstances.size() == 0) {
            return 0;
        } else {
            T* instance = myFreeInstances.back();
            myFreeInstances.pop_back();
            return instance;
        }
    }


    /** @brief Adds a free, reusable instance
     *
     * @param[in] instance An instance to add
     */
    void addFreeInstance(T* instance) {
        myFreeInstances.push_back(instance);
    }


    /** @brief Adds some free, reusable instances
     *
     * @param[in] instances A vector of instances to add
     */
    void addFreeInstances(const std::vector<T*> instances) {
        std::copy(instances.begin(), instances.end(),
                  std::back_inserter(myFreeInstances));
    }


private:
    /// @brief List of reusable instances
    std::vector<T*> myFreeInstances;

    /// @brief Information whether the stored instances shall be deleted
    bool myDeleteOnQuit;


};
