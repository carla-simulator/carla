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
/// @file    GNEChange_Attribute.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A network change in which the attribute of some object is modified
/****************************************************************************/
#pragma once
#include <config.h>

#include "GNEChange.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEChange_Attribute
 * @brief the function-object for an editing operation (abstract base)
 */
class GNEChange_Attribute : public GNEChange {
    FXDECLARE_ABSTRACT(GNEChange_Attribute)

public:
    /**@brief constructor
     * @param[in] ac The attribute-carrier to be modified
     * @param[in] key The attribute key
     * @param[in] value The new value
     */
    GNEChange_Attribute(GNEAttributeCarrier* ac, const SumoXMLAttr key, const std::string& value);

    /**@brief Constructor with custom origin value
     * @param[in] ac The attribute-carrier to be modified
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] customOrigValue custon original value
     */
    GNEChange_Attribute(GNEAttributeCarrier* ac, const SumoXMLAttr key, const std::string& value,
                        const std::string& customOrigValue);

    /// @brief Destructor
    ~GNEChange_Attribute();

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

    /// @brief force change
    void forceChange();

    /// @brief wether original and new value differ
    bool trueChange();

private:
    /**@brief the net to which all operations shall be applied
     * @note we are not responsible for the pointer
     */
    GNEAttributeCarrier* myAC;

    /// @brief The attribute name (or the original attribute if we're editing a disjoint attribute)
    const SumoXMLAttr myKey;

    /// @brief flag used to force set attributes
    bool myForceChange;

    /// @brief the original value
    const std::string myOrigValue;

    /// @brief the new value
    const std::string myNewValue;
};
