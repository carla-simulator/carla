/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUITexturesHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 08.03.2004
///
// Global storage for textures; manages and draws them
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <fx.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include "GUITexturesHelper.h"


// ===========================================================================
// definition of static variables
// ===========================================================================
std::map<std::string, int> GUITexturesHelper::myTextures;
bool GUITexturesHelper::myAllowTextures = true;


// ===========================================================================
// method definitions
// ===========================================================================
int
GUITexturesHelper::getMaxTextureSize() {
    int max;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    return max;
}


GUIGlID
GUITexturesHelper::add(FXImage* i) {
    GUIGlID id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 i->getWidth(), i->getHeight(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, i->getData());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}


void
GUITexturesHelper::drawTexturedBox(int which, double size) {
    drawTexturedBox(which, size, size, -size, -size);
}


void
GUITexturesHelper::drawTexturedBox(int which, double sizeX1, double sizeY1, double sizeX2, double sizeY2) {
    // first check that textures are allowed
    if (!myAllowTextures) {
        return;
    }
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST); // without DEPTH_TEST vehicles may be drawn below roads
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, which);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 1);
    glVertex2d(sizeX1, sizeY1);
    glTexCoord2f(0, 0);
    glVertex2d(sizeX1, sizeY2);
    glTexCoord2f(1, 1);
    glVertex2d(sizeX2, sizeY1);
    glTexCoord2f(1, 0);
    glVertex2d(sizeX2, sizeY2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}


int
GUITexturesHelper::getTextureID(const std::string& filename, const bool mirrorX) {
    if (myTextures.count(filename) == 0) {
        try {
            // load image
            FXImage* i = MFXImageHelper::loadImage(GUIMainWindow::getInstance()->getApp(), filename);
            if (mirrorX) {
                i->mirror(false, true);
            }
            MFXImageHelper::scalePower2(i, getMaxTextureSize());
            // Create GL structure using texture
            GUIGlID id = add(i);
            // delete texture after creating GL Structure
            delete i;
            myTextures[filename] = (int)id;
        } catch (InvalidArgument& e) {
            WRITE_ERROR("Could not load '" + filename + "'.\n" + e.what());
            myTextures[filename] = -1;
        }
    }
    return myTextures[filename];
}


void
GUITexturesHelper::clearTextures() {
    myTextures.clear();
}


/****************************************************************************/
