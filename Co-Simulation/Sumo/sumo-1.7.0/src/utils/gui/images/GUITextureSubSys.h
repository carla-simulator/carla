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
/// @file    GUITextureSubSys.h
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2016
///
// A class to manage gifs of SUMO
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include "GUITextures.h"
#include "GUITexturesHelper.h"

// ===========================================================================
// class definitions
// ===========================================================================

class GUITextureSubSys {
public:
    /**@brief Initiate GUITextureSubSys for textures
     * @param[in] a FOX Toolkit APP
     */
    static void initTextures(FXApp* a);

    /**@brief returns a texture previously defined in the enum GUITexture
     * @param[in] GUITexture code of texture to use
     */
    static GUIGlID getTexture(GUITexture which);

    /**@brief Reset textures
     * @note Necessary to avoid problems with textures (ej: white empty)
     */
    static void resetTextures();

    /// @brief close GUITextureSubSys
    static void close();

private:
    /// @brief constructor private because is called by the static function init(FXApp* a
    GUITextureSubSys(FXApp* a);

    /// @brief destructor
    ~GUITextureSubSys();

    /// @pointer to Fox App
    FXApp* myApp;

    /// @brief instance of GUITextureSubSys
    static GUITextureSubSys* myInstance;

    /// @brief vector with the Gifs
    std::map<GUITexture, GUIGlID> myTextures;
};
