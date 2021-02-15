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
/// @file    GNEChange_DataSet.h
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A network change in which a data set element is created or deleted
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_DataSet
 * A network change in which a data set element is created or deleted
 */
class GNEChange_DataSet : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_DataSet)

public:
    /**@brief Constructor for creating/deleting an data set element
     * @param[in] data set The data set element to be created/deleted
     * @param[in] forward Whether to create/delete (true/false)
     */
    GNEChange_DataSet(GNEDataSet* dataSet, bool forward);

    /// @brief Destructor
    ~GNEChange_DataSet();

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
    /**@brief full information regarding the data set element that is to be created/deleted
     * @note we assume shared responsibility for the pointer (via reference counting)
     */
    GNEDataSet* myDataSet;
};
