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
/// @file    GUILaneSpeedTrigger.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 26.04.2004
///
// Changes the speed allowed on a set of lanes (gui version)
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/common/ToString.h>
#include <utils/common/Command.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <guisim/GUINet.h>
#include <guisim/GUIEdge.h>
#include "GUILaneSpeedTrigger.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <gui/GUIApplicationWindow.h>
#include <microsim/logging/FunctionBinding.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <guisim/GUILaneSpeedTrigger.h>
#include <utils/gui/globjects/GLIncludes.h>


// ===========================================================================
// FOX callback mapping
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu - mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu)
GUILaneSpeedTriggerPopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_MANIP,         GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu::onCmdOpenManip),

};

// Object implementation
FXIMPLEMENT(GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu, GUIGLObjectPopupMenu, GUILaneSpeedTriggerPopupMenuMap, ARRAYNUMBER(GUILaneSpeedTriggerPopupMenuMap))


/* -------------------------------------------------------------------------
 * GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger - mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger) GUIManip_LaneSpeedTriggerMap[] = {
    FXMAPFUNC(SEL_COMMAND,  GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::MID_USER_DEF, GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdUserDef),
    FXMAPFUNC(SEL_UPDATE,   GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::MID_USER_DEF, GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onUpdUserDef),
    FXMAPFUNC(SEL_COMMAND,  GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::MID_PRE_DEF,  GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdPreDef),
    FXMAPFUNC(SEL_UPDATE,   GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::MID_PRE_DEF,  GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onUpdPreDef),
    FXMAPFUNC(SEL_COMMAND,  GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::MID_OPTION,   GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdChangeOption),
    FXMAPFUNC(SEL_COMMAND,  GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::MID_CLOSE,    GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdClose),
};

FXIMPLEMENT(GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger, GUIManipulator, GUIManip_LaneSpeedTriggerMap, ARRAYNUMBER(GUIManip_LaneSpeedTriggerMap))


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger - methods
 * ----------------------------------------------------------------------- */
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::GUIManip_LaneSpeedTrigger(
    GUIMainWindow& app,
    const std::string& name, GUILaneSpeedTrigger& o,
    int /*xpos*/, int /*ypos*/)
    : GUIManipulator(app, name, 0, 0),
      myParent(&app), myChosenValue(0), myChosenTarget(myChosenValue, nullptr, MID_OPTION),
      mySpeed(o.getDefaultSpeed()), mySpeedTarget(mySpeed),
      myObject(&o) {
    myChosenTarget.setTarget(this);
    FXVerticalFrame* f1 =
        new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);

    FXGroupBox* gp = new FXGroupBox(f1, "Change Speed",
                                    GROUPBOX_TITLE_LEFT | FRAME_RIDGE,
                                    0, 0, 0, 0,  4, 4, 1, 1, 2, 0);
    {
        // default
        FXHorizontalFrame* gf1 =
            new FXHorizontalFrame(gp, LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 10, 10, 5, 5);
        new FXRadioButton(gf1, "Default", &myChosenTarget, FXDataTarget::ID_OPTION + 0,
                          ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP,
                          0, 0, 0, 0,   2, 2, 0, 0);
    }
    {
        // loaded
        FXHorizontalFrame* gf0 =
            new FXHorizontalFrame(gp, LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 10, 10, 5, 5);
        new FXRadioButton(gf0, "Loaded", &myChosenTarget, FXDataTarget::ID_OPTION + 1,
                          ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP,
                          0, 0, 0, 0,   2, 2, 0, 0);
    }
    {
        // predefined
        FXHorizontalFrame* gf2 =
            new FXHorizontalFrame(gp, LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 10, 10, 5, 5);
        new FXRadioButton(gf2, "Predefined: ", &myChosenTarget, FXDataTarget::ID_OPTION + 2,
                          ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y,
                          0, 0, 0, 0,   2, 2, 0, 0);
        myPredefinedValues =
            new FXComboBox(gf2, 10, this, MID_PRE_DEF,
                           ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y | COMBOBOX_STATIC);
        myPredefinedValues->appendItem("20 km/h");
        myPredefinedValues->appendItem("40 km/h");
        myPredefinedValues->appendItem("60 km/h");
        myPredefinedValues->appendItem("80 km/h");
        myPredefinedValues->appendItem("100 km/h");
        myPredefinedValues->appendItem("120 km/h");
        myPredefinedValues->appendItem("140 km/h");
        myPredefinedValues->appendItem("160 km/h");
        myPredefinedValues->appendItem("180 km/h");
        myPredefinedValues->appendItem("200 km/h");
        myPredefinedValues->setNumVisible(5);
    }
    {
        // free
        FXHorizontalFrame* gf12 =
            new FXHorizontalFrame(gp, LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 10, 10, 5, 5);
        new FXRadioButton(gf12, "Free Entry: ", &myChosenTarget, FXDataTarget::ID_OPTION + 3,
                          ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y,
                          0, 0, 0, 0,   2, 2, 0, 0);
        myUserDefinedSpeed =
            new FXRealSpinner(gf12, 10, this, MID_USER_DEF,
                              LAYOUT_TOP | FRAME_SUNKEN | FRAME_THICK);
        //myUserDefinedSpeed->setFormatString("%.0f km/h");
        //myUserDefinedSpeed->setIncrements(1, 10, 10);
        myUserDefinedSpeed->setIncrement(10);
        myUserDefinedSpeed->setRange(0, 300);
        myUserDefinedSpeed->setValue(
            static_cast<GUILaneSpeedTrigger*>(myObject)->getDefaultSpeed() * 3.6);
    }
    new FXButton(f1, "Close", nullptr, this, MID_CLOSE,
                 BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, 30, 30, 4, 4);
    static_cast<GUILaneSpeedTrigger*>(myObject)->setOverriding(true);
}


GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::~GUIManip_LaneSpeedTrigger() {}


long
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdClose(FXObject*, FXSelector, void*) {
    destroy();
    return 1;
}


long
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdUserDef(FXObject*, FXSelector, void*) {
    mySpeed = (double)(myUserDefinedSpeed->getValue() / 3.6);
    static_cast<GUILaneSpeedTrigger*>(myObject)->setOverridingValue(mySpeed);
    myParent->updateChildren();
    return 1;
}


long
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onUpdUserDef(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myChosenValue != 3 ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    myParent->updateChildren();
    return 1;
}


long
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdPreDef(FXObject*, FXSelector, void*) {
    mySpeed = (double)(double)((myPredefinedValues->getCurrentItem() * 20 + 20) / 3.6);
    static_cast<GUILaneSpeedTrigger*>(myObject)->setOverridingValue(mySpeed);
    myParent->updateChildren();
    return 1;
}


long
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onUpdPreDef(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myChosenValue != 2 ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    myParent->updateChildren();
    return 1;
}


long
GUILaneSpeedTrigger::GUIManip_LaneSpeedTrigger::onCmdChangeOption(FXObject*, FXSelector, void*) {
    static_cast<GUILaneSpeedTrigger*>(myObject)->setOverriding(true);
    switch (myChosenValue) {
        case 0:
            mySpeed = (double) static_cast<GUILaneSpeedTrigger*>(myObject)->getDefaultSpeed();
            break;
        case 1:
            mySpeed = (double) static_cast<GUILaneSpeedTrigger*>(myObject)->getLoadedSpeed();
            break;
        case 2:
            mySpeed = (double)((myPredefinedValues->getCurrentItem() * 20 + 20) / 3.6);
            break;
        case 3:
            mySpeed = (double)(myUserDefinedSpeed->getValue() / 3.6);
            break;
        default:
            // hmmm, should not happen
            break;
    }
    static_cast<GUILaneSpeedTrigger*>(myObject)->setOverridingValue(mySpeed);
    myParent->updateChildren();
    if (myChosenValue == 1) {
        // !!! lock in between
        static_cast<GUILaneSpeedTrigger*>(myObject)->setOverriding(false);
    }
    return 1;
}



/* -------------------------------------------------------------------------
 * GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu - methods
 * ----------------------------------------------------------------------- */
GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu::GUILaneSpeedTriggerPopupMenu(
    GUIMainWindow& app, GUISUMOAbstractView& parent,
    GUIGlObject& o)
    : GUIGLObjectPopupMenu(app, parent, o) {}


GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu::~GUILaneSpeedTriggerPopupMenu() {}


long
GUILaneSpeedTrigger::GUILaneSpeedTriggerPopupMenu::onCmdOpenManip(FXObject*,
        FXSelector,
        void*) {
    static_cast<GUILaneSpeedTrigger*>(myObject)->openManipulator(
        *myApplication, *myParent);
    return 1;
}

// -------------------------------------------------------------------------
// GUILaneSpeedTrigger - methods
// -------------------------------------------------------------------------

GUILaneSpeedTrigger::GUILaneSpeedTrigger(
    const std::string& id, const std::vector<MSLane*>& destLanes,
    const std::string& aXMLFilename) :
    MSLaneSpeedTrigger(id, destLanes, aXMLFilename),
    GUIGlObject_AbstractAdd(GLO_VSS, id),
    myShowAsKMH(true), myLastValue(-1) {
    myFGPositions.reserve(destLanes.size());
    myFGRotations.reserve(destLanes.size());
    std::vector<MSLane*>::const_iterator i;
    for (i = destLanes.begin(); i != destLanes.end(); ++i) {
        const PositionVector& v = (*i)->getShape();
        myFGPositions.push_back(v.positionAtOffset(0));
        myBoundary.add(v.positionAtOffset(0));
        myFGRotations.push_back(-v.rotationDegreeAtOffset(0));
    }
}


GUILaneSpeedTrigger::~GUILaneSpeedTrigger() {}


GUIGLObjectPopupMenu*
GUILaneSpeedTrigger::getPopUpMenu(GUIMainWindow& app,
                                  GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUILaneSpeedTriggerPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildShowManipulatorPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUILaneSpeedTrigger::getParameterWindow(GUIMainWindow& app,
                                        GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("speed [m/s]", true,
                new FunctionBinding<GUILaneSpeedTrigger, double>(this, &GUILaneSpeedTrigger::getCurrentSpeed));
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUILaneSpeedTrigger::drawGL(const GUIVisualizationSettings& s) const {
    glPushName(getGlID());
    glPushMatrix();
    glTranslated(0, 0, getType());
    const double exaggeration = s.addSize.getExaggeration(s, this);
    for (int i = 0; i < (int)myFGPositions.size(); ++i) {
        const Position& pos = myFGPositions[i];
        double rot = myFGRotations[i];
        glPushMatrix();
        glTranslated(pos.x(), pos.y(), 0);
        glRotated(rot, 0, 0, 1);
        glTranslated(0, -1.5, 0);
        glScaled(exaggeration, exaggeration, 1);
        int noPoints = 9;
        if (s.scale > 25) {
            noPoints = (int)(9.0 + s.scale / 10.0);
            if (noPoints > 36) {
                noPoints = 36;
            }
        }
        glColor3d(1, 0, 0);
        GLHelper::drawFilledCircle((double) 1.3, noPoints);
        if (s.scale >= 5) {
            glTranslated(0, 0, .1);
            glColor3d(0, 0, 0);
            GLHelper::drawFilledCircle((double) 1.1, noPoints);
            // draw the speed string
            // not if scale to low
            // compute
            double value = (double) getCurrentSpeed();
            if (myShowAsKMH) {
                value *= 3.6f;
                if (((int) value + 1) % 10 == 0) {
                    value = (double)(((int) value + 1) / 10 * 10);
                }
            }
            if (value != myLastValue) {
                myLastValue = value;
                myLastValueString = toString<double>(myLastValue);
                std::string::size_type idx = myLastValueString.find('.');
                if (idx != std::string::npos) {
                    if (idx > myLastValueString.length()) {
                        idx = myLastValueString.length();
                    }
                    myLastValueString = myLastValueString.substr(0, idx);
                }
            }
            //draw
            glColor3d(1, 1, 0);
            glTranslated(0, 0, .1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // draw last value string
            GLHelper::drawText(myLastValueString.c_str(), Position(0, 0), .1, 1.2, RGBColor(255, 255, 0), 180);
        }
        glPopMatrix();
    }
    glPopMatrix();
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


Boundary
GUILaneSpeedTrigger::getCenteringBoundary() const {
    Boundary b(myBoundary);
    b.grow(20);
    return b;
}


GUIManipulator*
GUILaneSpeedTrigger::openManipulator(GUIMainWindow& app,
                                     GUISUMOAbstractView&) {
    GUIManip_LaneSpeedTrigger* gui =
        new GUIManip_LaneSpeedTrigger(app, getFullName(), *this, 0, 0);
    gui->create();
    gui->show();
    return gui;
}


/****************************************************************************/
