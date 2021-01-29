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
/// @file    GNEChange_DataInterval.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A network change in which a data interval element is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_DataInterval
 * A network change in which a data interval element is created or deleted
 */
class GNEChange_DataInterval : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_DataInterval)

public:
    /**@brief Constructor for creating/deleting an data interval element
     * @param[in] data interval The data interval element to be created/deleted
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_DataInterval(GNEDataInterval* dataInterval, bool forward);

    /// @brief Destructor
    ~GNEChange_DataInterval();

    /// @name inherited from GNEChange
    /// @{
    /// @brief get undo Name
    FXString undoName() const;

    /// @brief get Redo name
    FXString redoName() const;

    /// @brief undo action
    void undo();

    /// @brief redo action
    void redo();
    /// @}

private:
    /**@brief full information regarding the data interval element that is to be created/deleted
     * @note we assume shared responsibility for the pointer (via reference counting)
     */
    GNEDataInterval* myDataInterval;

    /// @brief pointer to data set parent
    GNEDataSet* myDataSetParent;
};
