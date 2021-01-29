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
/// @file    GNEReferenceCounter.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A class that counts references to itself
// We may wish to keep references to junctions/nodes either in the network or in the undoList
// to clean up properly we have to resort to reference counting
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class  GNEReferenceCounter
 */
class GNEReferenceCounter {
public:
    //// @brief constructor
    GNEReferenceCounter(): myCount(0) {}

    /// @brief return ID of object
    virtual const std::string& getID() const = 0;

    //// @brief destructor
    ~GNEReferenceCounter() {
        // If myCount is different of 0, means that references weren't removed correctly
        if (myCount != 0) {
            // cannot print id here, it already got destructed
            WRITE_ERROR("Attempt to delete instance of GNEReferenceCounter with count " + toString(myCount));
        }
    }

    /// @brief Decrease reference
    void decRef(const std::string& debugMsg = "") {
        // debugMsg only used for print debugging
#ifdef _DEBUG_REFERENCECOUNTER
        std::cout << "decRef (" + toString(myCount) + ") for " + getID() + ": " << debugMsg << "\n";
#else
        UNUSED_PARAMETER(debugMsg);
#endif
        // write error if decrement results into a negative count
        if (myCount < 1) {
            WRITE_ERROR("Attempt to decrement references below zero for instance of GNEReferenceCounter");
        }
        myCount--;
    }

    /// @brief Increarse reference
    void incRef(const std::string& debugMsg = "") {
        // debugMsg only used for print debugging
#ifdef _DEBUG_REFERENCECOUNTER
        std::cout << "incRef (" + toString(myCount) + ") for " + getID() + ": " << debugMsg << "\n";
#else
        UNUSED_PARAMETER(debugMsg);
#endif
        myCount++;
    }

    /// @brief check if object ins't referenced
    bool unreferenced() {
        return myCount == 0;
    }

private:
    /// @brief reference counter
    int myCount;
};
