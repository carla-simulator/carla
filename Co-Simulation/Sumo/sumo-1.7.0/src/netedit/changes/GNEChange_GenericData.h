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
/// @file    GNEChange_GenericData.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A network change in which a generic data set is created or deleted
/****************************************************************************/
#pragma once

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_GenericData
 * A network change in which a generic data set is created or deleted
 */
class GNEChange_GenericData : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_GenericData)

public:
    /**@brief Constructor for creating/deleting an generic data set
     * @param[in] generic data The generic data set to be created/deleted
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_GenericData(GNEGenericData* genericData, bool forward);

    /// @brief Destructor
    ~GNEChange_GenericData();

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
    /**@brief full information regarding the generic data set that is to be created/deleted
     * @note we assume shared responsibility for the pointer (via reference counting)
     */
    GNEGenericData* myGenericData;

    /// @brief data set parent
    GNEDataSet* myDataSetParent;

    /// @brief data interval parent
    GNEDataInterval* myDataIntervalParent;

    /// @brief reference to path vector
    const std::vector<GNEPathElements::PathElement>& myPath;
};

/****************************************************************************/
