//
// Copyright (C) 2005 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// author:      Daniel Willkomm
// part of:     framework implementation developed by tkn
// description: Class to store information about a nic for the
//              ConnectionManager module

#pragma once

#include "veins/base/connectionManager/NicEntry.h"

#include <map>
#include <vector>

namespace veins {

/**
 * @brief NicEntry is used by ConnectionManager to store the necessary
 * information for each nic
 *
 * @ingroup connectionManager
 * @author Daniel Willkomm
 * @sa ConnectionManager, NicEntry
 */
class VEINS_API NicEntryDebug : public NicEntry {
protected:
    /** @brief Number of in gates allocated for the nic so far*/
    int inCnt;

    /** @brief Number of out gates allocated for the nic so far */
    int outCnt;

    /** @brief Check for unknown free gates before next gate request.
     *
     * This flag is true after creation of the NicEntryDebug and assures
     * that it checks for already existing in- and out-gates from a previous
     * NicEntryDebug (which can happen if the same NIC is unregistered and
     * later again registered with a connection manager.
     */
    bool checkFreeGates;

    using GateStack = std::vector<cGate*>;
    /** @brief In Gates that were once used but are not connected now */
    GateStack freeInGates;

    /** @brief Out Gates that were once used but are not connected now */
    GateStack freeOutGates;

protected:
    /**
     * @brief Returns a free in gate of the nic
     *
     * This checks the list of free in gates, if one is available it is
     * returned. Otherwise, a new in gate is added to the nic.
     */
    cGate* requestInGate(void);

    /**
     * @brief Returns a free out gate of the nic
     *
     * returns a free out gate. If none is available it is created. See
     * NicEntry::requestInGate for a detailed description
     */
    cGate* requestOutGate(void);

    /**
     * @brief Collects all free gates with a certain pattern as names and puts
     * them on a stack.
     *
     * @param pattern The naming pattern of the gates, something like "out%d-%d"
     * where the first %d is filled with the nic id and the second with an
     * incrementing counter.
     * @param gates The gate stack in which to put the found gates.
     * @return the number of free gates found.
     */
    int collectGates(const char* pattern, GateStack& gates);

    /**
     * @brief Iterates over all existing gates of this NicEntries nic and host
     * and checks wether they are useable by this nic entry.
     *
     * This method is necessary if a nic was unregistered from its connection
     * manager during a simulation and is then again registered with one.
     * When unregistered the gates created by this NicEntryDebug are not deleted
     * so they can be collected and reused again when the nic is registered
     * again.
     */
    void collectFreeGates();

public:
    /**
     * @brief Constructor, initializes all members
     */
    NicEntryDebug(cComponent* owner)
        : NicEntry(owner)
        , inCnt(0)
        , outCnt(0)
        , checkFreeGates(true){};

    /**
     * @brief Removes all dynamically created out-/ingates.
     */
    ~NicEntryDebug() override
    {
    }

    /**
     * @brief Connect two nics
     *
     * Establish unidirectional connection with other nic
     *
     * @param other reference to remote nic (other NicEntry)
     *
     * This function acquires an in gate at the remote nic and an out
     * gate at this nic, connects the two and updates the freeInGate,
     * freeOutGate and outConns data sets.
     **/
    void connectTo(NicEntry* other) override;

    /**
     * @brief Disconnect two nics
     *
     * Release unidirectional connection with other nic
     *
     * @param other reference to remote nic (other NicEntry)
     **/
    void disconnectFrom(NicEntry* other) override;
};

} // namespace veins
