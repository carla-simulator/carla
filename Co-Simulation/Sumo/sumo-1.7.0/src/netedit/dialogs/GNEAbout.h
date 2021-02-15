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
/// @file    GNEAbout.h
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// The "About" - dialog for NETEDIT, (adapted from GUIDialog_AboutSUMO)
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEAbout
 * @brief The application's "About" - dialog
 */
class GNEAbout : public FXDialogBox {
public:
    /**@brief Constructor
     * @param[in] parent The parent window
     */
    GNEAbout(FXWindow* parent);

    /// @brief Destructor
    ~GNEAbout();

    /// @brief Creates the widget
    void create();

private:
    /// @brief Font for the widget
    FXFont* myHeadlineFont;
};
