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
/// @file    GUIUserIO.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2006
///
// Some OS-dependant functions to ease cliboard manipulation
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <fx.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIUserIO
 * @brief Some OS-dependant functions to ease cliboard manipulation
 *
 * This helper contains only one function by now. It is used to store a text
 *  snipplet permanently within Windows-clipboard. This method is necessary,
 *  because FOX only implements the Linux/UNIX-text copying scheme where
 *  a marked text is copied. This does not work as soon as the widget with
 *  the marked text is destroyed or when the selection is lost. Because this
 *  scheme differs very much from the way the clipboard is handled in windows,
 *  is is implemented "from scratch" herein.
 */
class GUIUserIO {
public:
    /** @brief Copies the given text to clipboard
     *
     * @param[in] app The application to use
     * @param[in] text The text to copy
     */
    static void copyToClipboard(const FXApp& app, const std::string& text);

    /** @brief Copies text from the clipboard
     *
     * @param[in] app The application to use
     */
    static std::string copyFromClipboard(const FXApp& app);

    static std::string clipped;

};
