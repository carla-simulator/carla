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
/// @file    GUICalibrator.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 26.04.2004
///
// Changes flow and speed on a set of lanes (gui version)
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
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <gui/GUIApplicationWindow.h>
#include <microsim/logging/FunctionBinding.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <guisim/GUICalibrator.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GUICalibrator.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUICalibrator::GUICalibratorPopupMenu - mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUICalibrator::GUICalibratorPopupMenu)
GUICalibratorPopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_MANIP,         GUICalibrator::GUICalibratorPopupMenu::onCmdOpenManip),

};

// Object implementation
FXIMPLEMENT(GUICalibrator::GUICalibratorPopupMenu, GUIGLObjectPopupMenu, GUICalibratorPopupMenuMap, ARRAYNUMBER(GUICalibratorPopupMenuMap))


/* -------------------------------------------------------------------------
 * GUICalibrator::GUIManip_Calibrator - mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUICalibrator::GUIManip_Calibrator) GUIManip_CalibratorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  GUICalibrator::GUIManip_Calibrator::MID_USER_DEF, GUICalibrator::GUIManip_Calibrator::onCmdUserDef),
    FXMAPFUNC(SEL_UPDATE,   GUICalibrator::GUIManip_Calibrator::MID_USER_DEF, GUICalibrator::GUIManip_Calibrator::onUpdUserDef),
    FXMAPFUNC(SEL_COMMAND,  GUICalibrator::GUIManip_Calibrator::MID_PRE_DEF,  GUICalibrator::GUIManip_Calibrator::onCmdPreDef),
    FXMAPFUNC(SEL_UPDATE,   GUICalibrator::GUIManip_Calibrator::MID_PRE_DEF,  GUICalibrator::GUIManip_Calibrator::onUpdPreDef),
    FXMAPFUNC(SEL_COMMAND,  GUICalibrator::GUIManip_Calibrator::MID_OPTION,   GUICalibrator::GUIManip_Calibrator::onCmdChangeOption),
    FXMAPFUNC(SEL_COMMAND,  GUICalibrator::GUIManip_Calibrator::MID_CLOSE,    GUICalibrator::GUIManip_Calibrator::onCmdClose),
};

FXIMPLEMENT(GUICalibrator::GUIManip_Calibrator, GUIManipulator, GUIManip_CalibratorMap, ARRAYNUMBER(GUIManip_CalibratorMap))


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUICalibrator::GUIManip_Calibrator - methods
 * ----------------------------------------------------------------------- */
GUICalibrator::GUIManip_Calibrator::GUIManip_Calibrator(
    GUIMainWindow& app,
    const std::string& name, GUICalibrator& o,
    int /*xpos*/, int /*ypos*/) :
    GUIManipulator(app, name, 0, 0),
    myParent(&app),
    myChosenValue(0),
    myChosenTarget(myChosenValue, nullptr, MID_OPTION),
    //mySpeed(o.getDefaultSpeed()),
    mySpeed(0),
    mySpeedTarget(mySpeed),
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
        myUserDefinedSpeed->setValue(0);
        //static_cast<GUICalibrator*>(myObject)->getDefaultSpeed() * 3.6);
    }
    new FXButton(f1, "Close", nullptr, this, MID_CLOSE,
                 BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, 30, 30, 4, 4);
    //static_cast<GUICalibrator*>(myObject)->setOverriding(true);
}


GUICalibrator::GUIManip_Calibrator::~GUIManip_Calibrator() {}


long
GUICalibrator::GUIManip_Calibrator::onCmdClose(FXObject*, FXSelector, void*) {
    destroy();
    return 1;
}


long
GUICalibrator::GUIManip_Calibrator::onCmdUserDef(FXObject*, FXSelector, void*) {
    //mySpeed = (double)(myUserDefinedSpeed->getValue() / 3.6);
    //static_cast<GUICalibrator*>(myObject)->setOverridingValue(mySpeed);
    //myParent->updateChildren();
    return 1;
}


long
GUICalibrator::GUIManip_Calibrator::onUpdUserDef(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myChosenValue != 3 ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    myParent->updateChildren();
    return 1;
}


long
GUICalibrator::GUIManip_Calibrator::onCmdPreDef(FXObject*, FXSelector, void*) {
    //mySpeed = (double)(double)((myPredefinedValues->getCurrentItem() * 20 + 20) / 3.6);
    //static_cast<GUICalibrator*>(myObject)->setOverridingValue(mySpeed);
    //myParent->updateChildren();
    return 1;
}


long
GUICalibrator::GUIManip_Calibrator::onUpdPreDef(FXObject* sender, FXSelector, void* ptr) {
    sender->handle(this,
                   myChosenValue != 2 ? FXSEL(SEL_COMMAND, ID_DISABLE) : FXSEL(SEL_COMMAND, ID_ENABLE),
                   ptr);
    myParent->updateChildren();
    return 1;
}


long
GUICalibrator::GUIManip_Calibrator::onCmdChangeOption(FXObject*, FXSelector, void*) {
    //static_cast<GUICalibrator*>(myObject)->setOverriding(true);
    //switch (myChosenValue) {
    //    case 0:
    //        mySpeed = (double) static_cast<GUICalibrator*>(myObject)->getDefaultSpeed();
    //        break;
    //    case 1:
    //        mySpeed = (double) static_cast<GUICalibrator*>(myObject)->getLoadedSpeed();
    //        break;
    //    case 2:
    //        mySpeed = (double)((myPredefinedValues->getCurrentItem() * 20 + 20) / 3.6);
    //        break;
    //    case 3:
    //        mySpeed = (double)(myUserDefinedSpeed->getValue() / 3.6);
    //        break;
    //    default:
    //        // hmmm, should not happen
    //        break;
    //}
    //static_cast<GUICalibrator*>(myObject)->setOverridingValue(mySpeed);
    //myParent->updateChildren();
    //if (myChosenValue == 1) {
    //    // !!! lock in between
    //    static_cast<GUICalibrator*>(myObject)->setOverriding(false);
    //}
    return 1;
}



/* -------------------------------------------------------------------------
 * GUICalibrator::GUICalibratorPopupMenu - methods
 * ----------------------------------------------------------------------- */
GUICalibrator::GUICalibratorPopupMenu::GUICalibratorPopupMenu(
    GUIMainWindow& app, GUISUMOAbstractView& parent,
    GUIGlObject& o)
    : GUIGLObjectPopupMenu(app, parent, o) {}


GUICalibrator::GUICalibratorPopupMenu::~GUICalibratorPopupMenu() {}


long
GUICalibrator::GUICalibratorPopupMenu::onCmdOpenManip(FXObject*,
        FXSelector,
        void*) {
    static_cast<GUICalibrator*>(myObject)->openManipulator(
        *myApplication, *myParent);
    return 1;
}


/* -------------------------------------------------------------------------
 * GUICalibrator - methods
 * ----------------------------------------------------------------------- */
GUICalibrator::GUICalibrator(MSCalibrator* calibrator) :
    GUIGlObject_AbstractAdd(GLO_CALIBRATOR, calibrator->getID()),
    myCalibrator(calibrator),
    myShowAsKMH(true) {
    const std::vector<MSLane*>& destLanes = calibrator->myEdge->getLanes();
    const MSLane* lane = calibrator->myLane;
    const double pos = calibrator->myPos;
    for (std::vector<MSLane*>::const_iterator i = destLanes.begin(); i != destLanes.end(); ++i) {
        if (lane == nullptr || (*i) == lane) {
            const PositionVector& v = (*i)->getShape();
            myFGPositions.push_back(v.positionAtOffset(pos));
            myBoundary.add(v.positionAtOffset(pos));
            myFGRotations.push_back(-v.rotationDegreeAtOffset(pos));
        }
    }
}


GUICalibrator::~GUICalibrator() {}


GUIGLObjectPopupMenu*
GUICalibrator::getPopUpMenu(GUIMainWindow& app,
                            GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUICalibratorPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    //buildShowManipulatorPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUICalibrator::getParameterWindow(GUIMainWindow& app,
                                  GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret;
    auto myCurrentStateInterval = myCalibrator->myCurrentStateInterval;
    if (myCalibrator->isActive()) {
        ret = new GUIParameterTableWindow(app, *this);
        // add items
        ret->mkItem("interval start", false, STEPS2TIME(myCurrentStateInterval->begin));
        ret->mkItem("interval end", false, STEPS2TIME(myCurrentStateInterval->end));
        ret->mkItem("aspired flow [veh/h]", false, myCurrentStateInterval->q);
        ret->mkItem("aspired speed", false, myCurrentStateInterval->v);
        ret->mkItem("current flow [veh/h]", true, new FunctionBinding<MSCalibrator, double>(myCalibrator, &MSCalibrator::currentFlow));
        ret->mkItem("current speed", true, new FunctionBinding<MSCalibrator, double>(myCalibrator, &MSCalibrator::currentSpeed));
        ret->mkItem("default speed", false, myCalibrator->myDefaultSpeed);
        ret->mkItem("required vehicles", true, new FunctionBinding<MSCalibrator, int>(myCalibrator, &MSCalibrator::totalWished));
        ret->mkItem("passed vehicles", true, new FunctionBinding<MSCalibrator, int>(myCalibrator, &MSCalibrator::passed));
        ret->mkItem("inserted vehicles", true, new FunctionBinding<MSCalibrator, int>(myCalibrator, &MSCalibrator::inserted));
        ret->mkItem("removed vehicles", true, new FunctionBinding<MSCalibrator, int>(myCalibrator, &MSCalibrator::removed));
        ret->mkItem("cleared in jam", true, new FunctionBinding<MSCalibrator, int>(myCalibrator, &MSCalibrator::clearedInJam));
    } else {
        ret = new GUIParameterTableWindow(app, *this);
        const std::string nextStart =
            (myCurrentStateInterval != myCalibrator->myIntervals.end() ?
             time2string(myCurrentStateInterval->begin) :
             "simulation end");
        ret->mkItem("inactive until", false, nextStart);
    }
    // close building
    ret->closeBuilding();
    return ret;
}


void
GUICalibrator::drawGL(const GUIVisualizationSettings& s) const {
    glPushName(getGlID());
    std::string flow = "-";
    std::string speed = "-";
    if (myCalibrator->isActive()) {
        auto myCurrentStateInterval = myCalibrator->myCurrentStateInterval;
        if (myCurrentStateInterval->v >= 0) {
            speed = toString(myCurrentStateInterval->v) + "m/s";
        }
        if (myCurrentStateInterval->q >= 0) {
            flow = toString((int)myCurrentStateInterval->q) + "v/h";
        }
    }
    const double exaggeration = s.addSize.getExaggeration(s, this);
    for (int i = 0; i < (int)myFGPositions.size(); ++i) {
        const Position& pos = myFGPositions[i];
        double rot = myFGRotations[i];
        glPushMatrix();
        glTranslated(pos.x(), pos.y(), getType());
        glRotated(rot, 0, 0, 1);
        glTranslated(0, 0, getType());
        glScaled(exaggeration, exaggeration, 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        glColor3d(1, .8f, 0);
        // base
        glVertex2d(0 - 1.4, 0);
        glVertex2d(0 - 1.4, 6);
        glVertex2d(0 + 1.4, 6);
        glVertex2d(0 + 1.4, 0);
        glVertex2d(0 - 1.4, 0);
        glVertex2d(0 + 1.4, 6);
        glEnd();

        // draw text
        if (s.scale * exaggeration >= 1.) {
            glTranslated(0, 0, .1);
            GLHelper::drawText("C", Position(0, 2), 0.1, 3, RGBColor::BLACK, 180);
            GLHelper::drawText(flow, Position(0, 4), 0.1, 0.7, RGBColor::BLACK, 180);
            GLHelper::drawText(speed, Position(0, 5), 0.1, 0.7, RGBColor::BLACK, 180);
        }
        glPopMatrix();
    }
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


Boundary
GUICalibrator::getCenteringBoundary() const {
    Boundary b(myBoundary);
    b.grow(20);
    return b;
}


GUIManipulator*
GUICalibrator::openManipulator(GUIMainWindow& app,
                               GUISUMOAbstractView&) {
    GUIManip_Calibrator* gui =
        new GUIManip_Calibrator(app, getFullName(), *this, 0, 0);
    gui->create();
    gui->show();
    return gui;
}


/****************************************************************************/
