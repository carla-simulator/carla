/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUITexturesHelper.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2006
///
// Global storage for textures; manages and draws them
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <utils/gui/globjects/GUIGlObject.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUITexturesHelper
 * @brief Global storage for textures; manages and draws them
 */
class GUITexturesHelper {
public:
    /// @brief return maximum number of pixels in x and y direction
    static int getMaxTextureSize();

    /// @brief Adds a texture to use
    static GUIGlID add(FXImage* i);

    /// @brief Draws a named texture as a box with the given size
    static void drawTexturedBox(int which, double size);

    /// @brief Draws a named texture as a rectangle with the given sizes
    static void drawTexturedBox(int which, double sizeX1, double sizeY1, double sizeX2, double sizeY2);

    /**@brief return texture id for the given filename (initialize on first use)
     * @note return -1 on failure
     */
    static int getTextureID(const std::string& filename, const bool mirrorX = false);

    /// @brief clears loaded textures
    static void clearTextures();

    /// @brief switch texture drawing on and off
    static void allowTextures(const bool val) {
        myAllowTextures = val;
    }

    /// @brief ask whether texture drawing is enabled
    static bool texturesAllowed() {
        return myAllowTextures;
    }

private:
    /// @brief mapping from image paths to decals (initialization on first use)
    static std::map<std::string, int> myTextures;

    /// @brief whether textures are drawn
    static bool myAllowTextures;
};
