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
/// @file    GUIParameterTracker.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A window which displays the time line of one (or more) value(s)
/****************************************************************************/
#include <config.h>

#include <string>
#include <fstream>
#include <utils/foxtools/MFXUtils.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/SUMOTime.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/div/GUIIOGlobals.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <foreign/fontstash/fontstash.h>
#include "GUIParameterTracker.h"
#include <utils/gui/globjects/GLIncludes.h>


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIParameterTracker) GUIParameterTrackerMap[] = {
    FXMAPFUNC(SEL_CONFIGURE, 0,                       GUIParameterTracker::onConfigure),
    FXMAPFUNC(SEL_PAINT,     0,                       GUIParameterTracker::onPaint),
    FXMAPFUNC(SEL_COMMAND,   MID_SIMSTEP,             GUIParameterTracker::onSimStep),
    FXMAPFUNC(SEL_COMMAND,   GUIParameterTracker::MID_AGGREGATIONINTERVAL, GUIParameterTracker::onCmdChangeAggregation),
    FXMAPFUNC(SEL_COMMAND,   GUIParameterTracker::MID_SAVE, GUIParameterTracker::onCmdSave),

};

// Macro for the GLTestApp class hierarchy implementation
FXIMPLEMENT(GUIParameterTracker, FXMainWindow, GUIParameterTrackerMap, ARRAYNUMBER(GUIParameterTrackerMap))


// ===========================================================================
// method definitions
// ===========================================================================
GUIParameterTracker::GUIParameterTracker(GUIMainWindow& app,
        const std::string& name)
    : FXMainWindow(app.getApp(), "Tracker", nullptr, nullptr, DECOR_ALL, 20, 20, 300, 200),
      myApplication(&app) {
    buildToolBar();
    app.addChild(this);
    FXVerticalFrame* glcanvasFrame = new FXVerticalFrame(this, FRAME_SUNKEN | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);
    myPanel = new GUIParameterTrackerPanel(glcanvasFrame, *myApplication, *this);
    setTitle(name.c_str());
    setIcon(GUIIconSubSys::getIcon(GUIIcon::APP_TRACKER));
}


GUIParameterTracker::~GUIParameterTracker() {
    myApplication->removeChild(this);
    for (std::vector<TrackerValueDesc*>::iterator i1 = myTracked.begin(); i1 != myTracked.end(); i1++) {
        delete (*i1);
    }
    // deleted by GUINet
    for (std::vector<GLObjectValuePassConnector<double>*>::iterator i2 = myValuePassers.begin(); i2 != myValuePassers.end(); i2++) {
        delete (*i2);
    }
    delete myToolBarDrag;
    delete myToolBar;
}


void
GUIParameterTracker::create() {
    FXMainWindow::create();
    myToolBarDrag->create();
}


void
GUIParameterTracker::buildToolBar() {
    myToolBarDrag = new FXToolBarShell(this, GUIDesignToolBar);
    myToolBar = new FXToolBar(this, myToolBarDrag, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_RAISED);
    new FXToolBarGrip(myToolBar, myToolBar, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
    // save button
    new FXButton(myToolBar, "\t\tSave the data...",
                 GUIIconSubSys::getIcon(GUIIcon::SAVE), this, GUIParameterTracker::MID_SAVE, GUIDesignButtonToolbar);
    // aggregation interval combo
    myAggregationInterval =
        new FXComboBox(myToolBar, 8, this, MID_AGGREGATIONINTERVAL,
                       GUIDesignComboBoxStatic);
    myAggregationInterval->appendItem("1s");
    myAggregationInterval->appendItem("1min");
    myAggregationInterval->appendItem("5min");
    myAggregationInterval->appendItem("15min");
    myAggregationInterval->appendItem("30min");
    myAggregationInterval->appendItem("60min");
    myAggregationInterval->setNumVisible(6);
}


void
GUIParameterTracker::addTracked(GUIGlObject& o, ValueSource<double>* src,
                                TrackerValueDesc* newTracked) {
    myTracked.push_back(newTracked);
    // build connection (is automatically set into an execution map)
    myValuePassers.push_back(new GLObjectValuePassConnector<double>(o, src, newTracked));
}


long
GUIParameterTracker::onConfigure(FXObject* sender, FXSelector sel, void* ptr) {
    myPanel->onConfigure(sender, sel, ptr);
    return FXMainWindow::onConfigure(sender, sel, ptr);
}


long
GUIParameterTracker::onPaint(FXObject* sender, FXSelector sel, void* ptr) {
    myPanel->onPaint(sender, sel, ptr);
    return FXMainWindow::onPaint(sender, sel, ptr);
}


long
GUIParameterTracker::onSimStep(FXObject*, FXSelector, void*) {
    update();
    return 1;
}


long
GUIParameterTracker::onCmdChangeAggregation(FXObject*, FXSelector, void*) {
    int index = myAggregationInterval->getCurrentItem();
    int aggInt = 0;
    switch (index) {
        case 0:
            aggInt = 1;
            break;
        case 1:
            aggInt = 60;
            break;
        case 2:
            aggInt = 60 * 5;
            break;
        case 3:
            aggInt = 60 * 15;
            break;
        case 4:
            aggInt = 60 * 30;
            break;
        case 5:
            aggInt = 60 * 60;
            break;
        default:
            throw 1;
            break;
    }
    for (std::vector<TrackerValueDesc*>::iterator i1 = myTracked.begin(); i1 != myTracked.end(); i1++) {
        (*i1)->setAggregationSpan(TIME2STEPS(aggInt));
    }
    return 1;
}


long
GUIParameterTracker::onCmdSave(FXObject*, FXSelector, void*) {
    FXString file = MFXUtils::getFilename2Write(this, "Save Data", ".csv", GUIIconSubSys::getIcon(GUIIcon::EMPTY), gCurrentFolder);
    if (file == "") {
        return 1;
    }
    try {
        OutputDevice& dev = OutputDevice::getDevice(file.text());
        // write header
        std::vector<TrackerValueDesc*>::iterator i;
        dev << "# ";
        for (i = myTracked.begin(); i != myTracked.end(); ++i) {
            if (i != myTracked.begin()) {
                dev << ';';
            }
            TrackerValueDesc* tvd = *i;
            dev << tvd->getName();
        }
        dev << '\n';
        // count entries
        int max = 0;
        for (i = myTracked.begin(); i != myTracked.end(); ++i) {
            TrackerValueDesc* tvd = *i;
            int sizei = (int)tvd->getAggregatedValues().size();
            if (max < sizei) {
                max = sizei;
            }
            tvd->unlockValues();
        }
        // write entries
        for (int j = 0; j < max; j++) {
            for (i = myTracked.begin(); i != myTracked.end(); ++i) {
                if (i != myTracked.begin()) {
                    dev << ';';
                }
                TrackerValueDesc* tvd = *i;
                dev << tvd->getAggregatedValues()[j];
                tvd->unlockValues();
            }
            dev << '\n';
        }
        dev.close();
    } catch (IOError& e) {
        FXMessageBox::error(this, MBOX_OK, "Storing failed!", "%s", e.what());
    }
    return 1;
}


/* -------------------------------------------------------------------------
 * GUIParameterTracker::GUIParameterTrackerPanel-methods
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUIParameterTracker::GUIParameterTrackerPanel) GUIParameterTrackerPanelMap[] = {
    FXMAPFUNC(SEL_CONFIGURE, 0, GUIParameterTracker::GUIParameterTrackerPanel::onConfigure),
    FXMAPFUNC(SEL_PAINT,     0, GUIParameterTracker::GUIParameterTrackerPanel::onPaint),

};

// Macro for the GLTestApp class hierarchy implementation
FXIMPLEMENT(GUIParameterTracker::GUIParameterTrackerPanel, FXGLCanvas, GUIParameterTrackerPanelMap, ARRAYNUMBER(GUIParameterTrackerPanelMap))



GUIParameterTracker::GUIParameterTrackerPanel::GUIParameterTrackerPanel(
    FXComposite* c, GUIMainWindow& app,
    GUIParameterTracker& parent)
    : FXGLCanvas(c, app.getGLVisual(), app.getBuildGLCanvas(), (FXObject*) nullptr, (FXSelector) 0, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 300, 200),
      myParent(&parent), myApplication(&app) {}


GUIParameterTracker::GUIParameterTrackerPanel::~GUIParameterTrackerPanel() {}


void
GUIParameterTracker::GUIParameterTrackerPanel::drawValues() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);
    int run = 0;
    for (std::vector<TrackerValueDesc*>::iterator i = myParent->myTracked.begin(); i != myParent->myTracked.end(); i++) {
        TrackerValueDesc* desc = *i;
        drawValue(*desc,
                  (double) myWidthInPixels / (double) myParent->myTracked.size() * (double) run);
        run++;
    }
}


void
GUIParameterTracker::GUIParameterTrackerPanel::drawValue(TrackerValueDesc& desc,
        double /*namePos*/) {
    const double fontWidth = 0.1 * 300. / myWidthInPixels;
    const double fontHeight = 0.1 * 300. /  myHeightInPixels;
    //
    // apply scaling
    glPushMatrix();

    // apply the positiopn offset of the display
    glScaled(0.8, 0.8, 1);
    // apply value range scaling
    double ys = (double) 2.0 / (double) desc.getRange();
    glScaled(1.0, ys, 1.0);
    glTranslated(-1.0, -desc.getYCenter(), 0);

    // set color
    const unsigned char red = desc.getColor().red();
    const unsigned char green = desc.getColor().green();
    const unsigned char blue = desc.getColor().blue();
    // draw value bounderies
    // draw minimum boundary
    glBegin(GL_LINES);
    glVertex2d(0, desc.getMin());
    glVertex2d(2.0, desc.getMin());
    glEnd();
    glBegin(GL_LINES);
    glVertex2d(0, desc.getMax());
    glVertex2d(2.0, desc.getMax());
    glEnd();
    glColor4ub(red, green, blue, 77);
    for (int a = 1; a < 6; a++) {
        const double yp = desc.getRange() / 6.0 * (double) a + desc.getMin();
        glBegin(GL_LINES);
        glVertex2d(0, yp);
        glVertex2d(2.0, yp);
        glEnd();
    }
    const std::vector<double>& values = desc.getAggregatedValues();
    double latest = 0;
    if (values.size() < 2) {
        glPopMatrix();
        desc.unlockValues();
        return;
    } else {
        latest = values.back();
        // init values
        const double xStep = 2.0 / (double) values.size();
        std::vector<double>::const_iterator i = values.begin();
        double yp = (*i);
        double xp = 0;
        i++;
        glColor4ub(red, green, blue, 255);
        for (; i != values.end(); i++) {
            double yn = (*i);
            double xn = xp + xStep;
            glBegin(GL_LINES);
            glVertex2d(xp, yp);
            glVertex2d(xn, yn);
            glEnd();
            yp = yn;
            xp = xn;
        }
        desc.unlockValues();
        glPopMatrix();
    }

    // draw value bounderies and descriptions
    glColor3b(red, green, blue);

    // draw min time
    SUMOTime beginStep = desc.getRecordingBegin();
    std::string begStr = time2string(beginStep);
    double w = 50 / myWidthInPixels;
    glTranslated(-0.8 - w / 2., -0.88, 0);
    GLHelper::drawText(begStr, Position(0, 0), 1, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
    glTranslated(0.8 + w / 2., 0.88, 0);

    // draw max time
    glTranslated(0.75, -0.88, 0);
    GLHelper::drawText(time2string(beginStep + static_cast<SUMOTime>(values.size() * desc.getAggregationSpan())),
                       Position(0, 0), 1, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
    glTranslated(-0.75, 0.88, 0);

    // draw min value
    glTranslated(-0.98, -0.82, 0);
    GLHelper::drawText(toString(desc.getMin()), Position(0, 0), 1, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
    glTranslated(0.98, 0.82, 0);

    // draw max value
    glTranslated(-0.98, 0.78, 0);
    GLHelper::drawText(toString(desc.getMax()), Position(0, 0), 1, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
    glTranslated(0.98, -0.78, 0);

    // draw name
    glTranslated(-0.98, .92, 0);
    GLHelper::drawText(desc.getName(), Position(0, 0), 1, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
    glTranslated(0.98, -.92, 0);

    // draw current value (with contrasting color)
    double p = (double) 0.8 -
               ((double) 1.6 / (desc.getMax() - desc.getMin()) * (latest - desc.getMin()));
    glTranslated(-0.98, -(p + .02), 0);
    GLHelper::drawText(toString(latest), Position(0, 0), 1, fontHeight, RGBColor::RED, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
    glTranslated(0.98, p + .02, 0);

}


long
GUIParameterTracker::GUIParameterTrackerPanel::onConfigure(FXObject*,
        FXSelector, void*) {
    if (makeCurrent()) {
        myWidthInPixels = myParent->getWidth();
        myHeightInPixels = myParent->getHeight();
        if (myWidthInPixels != 0 && myHeightInPixels != 0) {
            glViewport(0, 0, myWidthInPixels - 1, myHeightInPixels - 1);
            glClearColor(1.0, 1.0, 1.0, 1);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glDisable(GL_COLOR_MATERIAL);
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        makeNonCurrent();
    }
    return 1;
}


long
GUIParameterTracker::GUIParameterTrackerPanel::onPaint(FXObject*,
        FXSelector, void*) {
    if (!isEnabled()) {
        return 1;
    }
    if (makeCurrent()) {
        myWidthInPixels = getWidth();
        myHeightInPixels = getHeight();
        if (myWidthInPixels != 0 && myHeightInPixels != 0) {
            glViewport(0, 0, myWidthInPixels - 1, myHeightInPixels - 1);
            glClearColor(1.0, 1.0, 1.0, 1);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glDisable(GL_COLOR_MATERIAL);
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // draw
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            drawValues();
            swapBuffers();
        }
        makeNonCurrent();
    }
    return 1;
}


/****************************************************************************/
