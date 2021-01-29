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
/// @file    GUITextureSubSys.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2016
///
// Helper for Gifs loading and usage
/****************************************************************************/
#include <config.h>

#include <fx.h>
#include <cassert>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>

#include "GUITextureSubSys.h"

#include "GNETexture_Lock.cpp"
#include "GNETexture_LockSelected.cpp"
#include "GNETexture_Empty.cpp"
#include "GNETexture_EmptySelected.cpp"
#include "GNETexture_TLS.cpp"
#include "GNETexture_E3.cpp"
#include "GNETexture_E3Selected.cpp"
#include "GNETexture_Rerouter.cpp"
#include "GNETexture_RerouterSelected.cpp"
#include "GNETexture_RouteProbe.cpp"
#include "GNETexture_RouteProbeSelected.cpp"
#include "GNETexture_Vaporizer.cpp"
#include "GNETexture_VaporizerSelected.cpp"
#include "GNETexture_VariableSpeedSign.cpp"
#include "GNETexture_VariableSpeedSignSelected.cpp"
#include "GNETexture_NotMoving.cpp"
#include "GNETexture_NotMovingSelected.cpp"
#include "GNETexture_LaneBus.cpp"
#include "GNETexture_LanePedestrian.cpp"
#include "GNETexture_LaneBike.cpp"


// ===========================================================================
// static member variable definitions
// ===========================================================================

GUITextureSubSys* GUITextureSubSys::myInstance = nullptr;

// ===========================================================================
// member definitions
// ===========================================================================

GUITextureSubSys::GUITextureSubSys(FXApp* a) :
    myApp(a) {
    // Fill map of textures
    myTextures[GNETEXTURE_E3] = 0;
    myTextures[GNETEXTURE_E3SELECTED] = 0;
    myTextures[GNETEXTURE_EMPTY] = 0;
    myTextures[GNETEXTURE_EMPTYSELECTED] = 0;
    myTextures[GNETEXTURE_LOCK] = 0;
    myTextures[GNETEXTURE_LOCKSELECTED] = 0;
    myTextures[GNETEXTURE_NOTMOVING] = 0;
    myTextures[GNETEXTURE_NOTMOVINGSELECTED] = 0;
    myTextures[GNETEXTURE_REROUTER] = 0;
    myTextures[GNETEXTURE_REROUTERSELECTED] = 0;
    myTextures[GNETEXTURE_ROUTEPROBE] = 0;
    myTextures[GNETEXTURE_ROUTEPROBESELECTED] = 0;
    myTextures[GNETEXTURE_TLS] = 0;
    myTextures[GNETEXTURE_VAPORIZER] = 0;
    myTextures[GNETEXTURE_VAPORIZERSELECTED] = 0;
    myTextures[GNETEXTURE_VARIABLESPEEDSIGN] = 0;
    myTextures[GNETEXTURE_VARIABLESPEEDSIGNSELECTED] = 0;
    myTextures[GNETEXTURE_LANEBIKE] = 0;
    myTextures[GNETEXTURE_LANEBUS] = 0;
    myTextures[GNETEXTURE_LANEPEDESTRIAN] = 0;
}


GUITextureSubSys::~GUITextureSubSys() {
}


void
GUITextureSubSys::initTextures(FXApp* a) {
    assert(myInstance == 0);
    myInstance = new GUITextureSubSys(a);
}


GUIGlID
GUITextureSubSys::getTexture(GUITexture which) {
    // If texture isn't loaded, load it
    if (myInstance->myTextures.at(which) == 0) {
        switch (which) {
            case GNETEXTURE_E3 :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_E3, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_E3SELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_E3Selected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_EMPTY :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_Empty, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_EMPTYSELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_EmptySelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_LOCK :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_Lock, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_LOCKSELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_LockSelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_NOTMOVING :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_NotMoving, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_NOTMOVINGSELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_NotMovingSelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_REROUTER :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_Rerouter, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_REROUTERSELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_RerouterSelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_ROUTEPROBE :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_RouteProbe, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_ROUTEPROBESELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_RouteProbeSelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_TLS :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_TLS, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_VAPORIZER :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_Vaporizer, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_VAPORIZERSELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_VaporizerSelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_VARIABLESPEEDSIGN :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_VariableSpeedSign, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_VARIABLESPEEDSIGNSELECTED :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_VariableSpeedSignSelected, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_LANEBIKE :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_LaneBike, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_LANEBUS :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_LaneBus, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            case GNETEXTURE_LANEPEDESTRIAN :
                myInstance->myTextures[which] = GUITexturesHelper::add(new FXGIFImage(myInstance->myApp, GNETexture_LanePedestrian, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP));
                break;
            default:
                throw ProcessError("Undefined texture");
        }
    }
    // Return GLID associated to the texture
    return myInstance->myTextures.at(which);
}


void
GUITextureSubSys::resetTextures() {
    // Reset all textures
    GUITexturesHelper::clearTextures();
    for (auto& i : myInstance->myTextures) {
        i.second = 0;
    }
}


void
GUITextureSubSys::close() {
    delete myInstance;
    myInstance = nullptr;
}


/****************************************************************************/
