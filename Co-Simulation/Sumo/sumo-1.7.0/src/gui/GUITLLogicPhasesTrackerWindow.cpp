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
/// @file    GUITLLogicPhasesTrackerWindow.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Oct/Nov 2003
///
// A window displaying the phase diagram of a tl-logic
/****************************************************************************/
#include <config.h>

#include <vector>
#include <iostream>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/gui/div/GLHelper.h>
#include "GUITLLogicPhasesTrackerWindow.h"
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/MSLink.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <guisim/GUITrafficLightLogicWrapper.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/gui/div/GUIDesigns.h>
#include <foreign/fontstash/fontstash.h>
#include <utils/gui/globjects/GLIncludes.h>


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel-callbacks
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel) GUITLLogicPhasesTrackerPanelMap[] = {
    FXMAPFUNC(SEL_CONFIGURE, 0, GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel::onConfigure),
    FXMAPFUNC(SEL_PAINT,     0, GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel::onPaint),

};

// Macro for the GLTestApp class hierarchy implementation
FXIMPLEMENT(GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel, FXGLCanvas, GUITLLogicPhasesTrackerPanelMap, ARRAYNUMBER(GUITLLogicPhasesTrackerPanelMap))



/* -------------------------------------------------------------------------
 * GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel-methods
 * ----------------------------------------------------------------------- */
GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel::GUITLLogicPhasesTrackerPanel(
    FXComposite* c, GUIMainWindow& app,
    GUITLLogicPhasesTrackerWindow& parent)
    : FXGLCanvas(c, app.getGLVisual(), app.getBuildGLCanvas(), (FXObject*) nullptr, (FXSelector) 0, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y/*, 0, 0, 300, 200*/),
      myParent(&parent) {}


GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel::~GUITLLogicPhasesTrackerPanel() {}


long
GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel::onConfigure(
    FXObject*, FXSelector, void*) {
    if (makeCurrent()) {
        int widthInPixels = getWidth();
        int heightInPixels = getHeight();
        if (widthInPixels != 0 && heightInPixels != 0) {
            glViewport(0, 0, widthInPixels - 1, heightInPixels - 1);
            glClearColor(0, 0, 0, 1);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glDisable(GL_COLOR_MATERIAL);
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    return 1;
}


long
GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerPanel::onPaint(
    FXObject*, FXSelector, void*) {
    if (!isEnabled()) {
        return 1;
    }
    if (makeCurrent()) {
        int widthInPixels = getWidth();
        int heightInPixels = getHeight();
        if (widthInPixels != 0 && heightInPixels != 0) {
            glViewport(0, 0, widthInPixels - 1, heightInPixels - 1);
            glClearColor(0, 0, 0, 1);
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
            myParent->drawValues(*this);
            swapBuffers();
        }
        makeNonCurrent();
    }
    return 1;
}



/* -------------------------------------------------------------------------
 * GUITLLogicPhasesTrackerWindow - FOX callback mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUITLLogicPhasesTrackerWindow) GUITLLogicPhasesTrackerWindowMap[] = {
    FXMAPFUNC(SEL_CONFIGURE, 0,           GUITLLogicPhasesTrackerWindow::onConfigure),
    FXMAPFUNC(SEL_PAINT,     0,           GUITLLogicPhasesTrackerWindow::onPaint),
    FXMAPFUNC(SEL_COMMAND,   MID_SIMSTEP, GUITLLogicPhasesTrackerWindow::onSimStep),

};

FXIMPLEMENT(GUITLLogicPhasesTrackerWindow, FXMainWindow, GUITLLogicPhasesTrackerWindowMap, ARRAYNUMBER(GUITLLogicPhasesTrackerWindowMap))


/* -------------------------------------------------------------------------
 * GUITLLogicPhasesTrackerWindow-methods
 * ----------------------------------------------------------------------- */
GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerWindow(
    GUIMainWindow& app,
    MSTrafficLightLogic& logic, GUITrafficLightLogicWrapper& wrapper,
    ValueSource<std::pair<SUMOTime, MSPhaseDefinition> >* src)
    : FXMainWindow(app.getApp(), "TLS-Tracker", nullptr, nullptr, DECOR_ALL,
                   20, 20, 300, 200),
      myApplication(&app), myTLLogic(&logic), myAmInTrackingMode(true) {
    // build the toolbar
    myToolBarDrag = new FXToolBarShell(this, GUIDesignToolBar);
    myToolBar = new FXToolBar(this, myToolBarDrag, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_RAISED);
    new FXToolBarGrip(myToolBar, myToolBar, FXToolBar::ID_TOOLBARGRIP, GUIDesignToolBarGrip);
    // interval manipulation
    myBeginOffset = new FXRealSpinner(myToolBar, 10, this, MID_SIMSTEP, LAYOUT_TOP | FRAME_SUNKEN | FRAME_THICK);
    //myBeginOffset->setFormatString("%.0f");
    //myBeginOffset->setIncrements(1, 10, 100);
    myBeginOffset->setIncrement(10);
    myBeginOffset->setRange(60, 3600);
    myBeginOffset->setValue(240);
    new FXLabel(myToolBar, "(s)", nullptr, LAYOUT_CENTER_Y);
    //
    myConnector = new GLObjectValuePassConnector<std::pair<SUMOTime, MSPhaseDefinition> >(wrapper, src, this);
    app.addChild(this);
    for (int i = 0; i < (int)myTLLogic->getLinks().size(); ++i) {
        myLinkNames.push_back(toString<int>(i));
    }
    FXVerticalFrame* glcanvasFrame =
        new FXVerticalFrame(this,
                            FRAME_SUNKEN | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                            0, 0, 0, 0, 0, 0, 0, 0);
    myPanel = new
    GUITLLogicPhasesTrackerPanel(glcanvasFrame, *myApplication, *this);
    setTitle((logic.getID() + " - " + logic.getProgramID() + " - tracker").c_str());
    setIcon(GUIIconSubSys::getIcon(GUIIcon::APP_TLSTRACKER));
    setHeight((FXint)(myTLLogic->getLinks().size() * 20 + 30 + 8 + 30));
    setWidth(700);
}


GUITLLogicPhasesTrackerWindow::GUITLLogicPhasesTrackerWindow(
    GUIMainWindow& app,
    MSTrafficLightLogic& logic, GUITrafficLightLogicWrapper& /*wrapper*/,
    const MSSimpleTrafficLightLogic::Phases& /*phases*/)
    : FXMainWindow(app.getApp(), "TLS-Tracker", nullptr, nullptr, DECOR_ALL,
                   20, 20, 300, 200),
      myApplication(&app), myTLLogic(&logic), myAmInTrackingMode(false),
      myToolBarDrag(nullptr), myBeginOffset(nullptr) {
    myConnector = nullptr;
    setTitle("TLS-Tracker");
    app.addChild(this);
    for (int i = 0; i < (int)myTLLogic->getLinks().size(); ++i) {
        myLinkNames.push_back(toString<int>(i));
    }
    FXVerticalFrame* glcanvasFrame =
        new FXVerticalFrame(this,
                            FRAME_SUNKEN | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                            0, 0, 0, 0, 0, 0, 0, 0);
    myPanel = new
    GUITLLogicPhasesTrackerPanel(glcanvasFrame, *myApplication, *this);
    setTitle((logic.getID() + " - " + logic.getProgramID() + " - tracker").c_str());
    setIcon(GUIIconSubSys::getIcon(GUIIcon::APP_TLSTRACKER));
    setHeight((FXint)(myTLLogic->getLinks().size() * 20 + 30 + 8));
    setWidth(700);
}


GUITLLogicPhasesTrackerWindow::~GUITLLogicPhasesTrackerWindow() {
    myApplication->removeChild(this);
    delete myConnector;
    // just to quit cleanly on a failure
    if (myLock.locked()) {
        myLock.unlock();
    }
    delete myToolBarDrag;
}


void
GUITLLogicPhasesTrackerWindow::create() {
    FXMainWindow::create();
    if (myToolBarDrag != nullptr) {
        myToolBarDrag->create();
    }
}


void
GUITLLogicPhasesTrackerWindow::drawValues(GUITLLogicPhasesTrackerPanel& caller) {
    // compute what shall be shown (what is visible)
    myFirstPhase2Show = 0;
    myFirstPhaseOffset = 0;
    SUMOTime leftOffset = 0;
    myFirstTime2Show = 0;
    if (!myAmInTrackingMode) {
        myPhases.clear();
        myDurations.clear();
        // insert phases
        MSSimpleTrafficLightLogic* simpleTLLogic = dynamic_cast<MSSimpleTrafficLightLogic*>(myTLLogic);
        if (simpleTLLogic == nullptr) {
            return;
        }
        const MSSimpleTrafficLightLogic::Phases& phases = simpleTLLogic->getPhases();
        MSSimpleTrafficLightLogic::Phases::const_iterator j;
        myLastTime = 0;
        myBeginTime = 0;
        for (j = phases.begin(); j != phases.end(); ++j) {
            myPhases.push_back(*(*j));
            myDurations.push_back((*j)->duration);
            myLastTime += (*j)->duration;
        }
        if (myLastTime <= myBeginTime) {
            WRITE_ERROR("Overflow in time computation occurred.");
            return;
        }
    } else {
        SUMOTime beginOffset = TIME2STEPS(myBeginOffset->getValue());
        myBeginTime = myLastTime - beginOffset;
        myFirstTime2Show = myBeginTime;
        // check whether no phases are known at all
        if (myDurations.size() != 0) {
            SUMOTime durs = 0;
            int phaseOffset = (int)myDurations.size() - 1;
            DurationsVector::reverse_iterator i = myDurations.rbegin();
            while (i != myDurations.rend()) {
                if (durs + (*i) > beginOffset) {
                    myFirstPhase2Show = phaseOffset;
                    myFirstPhaseOffset = (durs + (*i)) - beginOffset;
                    break;
                }
                durs += (*i);
                phaseOffset--;
                ++i;
            }
            if (i == myDurations.rend()) {
                // there are too few information stored;
                myFirstPhase2Show = 0;
                myFirstPhaseOffset = 0;
                leftOffset = beginOffset - durs;
            }
        }
    }
    // begin drawing
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(-1, -1, 0);
    glScaled(2, 2, 1);
    glDisable(GL_TEXTURE_2D);
    // draw the horizontal lines dividing the signal groups
    glColor3d(1, 1, 1);
    // compute some values needed more than once
    const double height = (double) caller.getHeight();
    const double width = (double) caller.getWidth();
    const double barWidth = MAX2(1.0, width - 31);
    const double fontHeight = 0.08 * 300. / height;
    const double fontWidth = 0.08 * 300. / width;
    const double h9 = ((double) 9 / height);
    const double h10 = ((double) 10 / height);
    const double h11 = ((double) 11 / height);
    const double h16 = ((double) 16 / height);
    const double h20 = ((double) 20 / height);
    // draw the link names and the lines dividing them
    double h = (double)(1.0 - h10);
    double h2 = 12;
    for (int i = 0; i < (int)myTLLogic->getLinks().size() + 1; ++i) {
        // draw the bar
        glBegin(GL_LINES);
        glVertex2d(0, h);
        glVertex2d((double)(30. / width), h);
        glEnd();
        // draw the name
        if (i < (int)myTLLogic->getLinks().size()) {
            glTranslated(0, h - h20, 0);
            GLHelper::drawText(myLinkNames[i], Position(0, 0), 1, fontHeight, RGBColor::WHITE, 0, FONS_ALIGN_LEFT | FONS_ALIGN_BOTTOM, fontWidth);
            glTranslated(0, -h + h20, 0);
            h2 += 20;
        }
        h -= h20;
    }
    glBegin(GL_LINES);
    glVertex2d(0, h + h20);
    glVertex2d(1.0, h + h20);
    glEnd();

    // draw the names closure (vertical line)
    h += (double) 20 / height;
    glColor3d(1, 1, 1);
    glBegin(GL_LINES);
    glVertex2d((double) 30 / width, 1.0);
    glVertex2d((double) 30 / width, h);
    glEnd();


    // draw the phases
    // disable value addition while drawing
    myLock.lock();
    // determine the initial offset
    double x = ((double) 31. / width);
    double ta = (double) leftOffset / width;
    ta *= (double)((barWidth / ((double)(myLastTime - myBeginTime))));
    x += ta;

    // and the initial phase information
    PhasesVector::iterator pi = myPhases.begin() + myFirstPhase2Show;

    SUMOTime fpo = myFirstPhaseOffset;

    // start drawing
    for (DurationsVector::iterator pd = myDurations.begin() + myFirstPhase2Show; pd != myDurations.end(); ++pd) {
        SUMOTime i = 30;
        // the first phase may be drawn incompletely
        SUMOTime duration = *pd - fpo;
        // compute the heigh and the width of the phase
        h = (double)(1.0 - h10);
        double a = (double) duration / width;
        a *= (double)((barWidth / ((double)(myLastTime - myBeginTime))));
        const double x2 = x + a;

        // go through the links
        for (int j = 0; j < (int) myTLLogic->getLinks().size(); ++j) {
            // determine the current link's color
            LinkState state = pi->getSignalState(j);
            // draw the bar (red is drawn as a line)
            GLHelper::setColor(GUIVisualizationSettings::getLinkColor(state));
            switch (state) {
                case LINKSTATE_TL_RED:
                case LINKSTATE_TL_REDYELLOW:
                    // draw a thin line
                    glBegin(GL_QUADS);
                    glVertex2d(x, h - h11);
                    glVertex2d(x, h - h9);
                    glVertex2d(x2, h - h9);
                    glVertex2d(x2, h - h11);
                    glEnd();
                    break;
                default:
                    // draw a thick block
                    glBegin(GL_QUADS);
                    glVertex2d(x, h - h16);
                    glVertex2d(x, h);
                    glVertex2d(x2, h);
                    glVertex2d(x2, h - h16);
                    glEnd();
                    break;
            }
            // proceed to next link
            h -= h20;
        }
        // proceed to next phase
        i += duration;
        ++pi;
        x = x2;
        // all further phases are drawn in full
        fpo = 0;
    }
    // allow value addition
    myLock.unlock();

    glColor3d(1, 1, 1);
    if (myPhases.size() != 0) {
        SUMOTime tickDist = TIME2STEPS(10);
        // patch distances - hack
        double t = myBeginOffset != nullptr ? (double) myBeginOffset->getValue() : STEPS2TIME(myLastTime - myBeginTime);
        while (t > barWidth / 4.) {
            tickDist += TIME2STEPS(10);
            t -= (double)(barWidth / 4.);
        }
        // draw time information
        //h = (double)(myTLLogic->getLinks().size() * 20 + 12);
        double glh = (double)(1.0 - myTLLogic->getLinks().size() * h20 - h10);
        // current begin time
        // time ticks
        SUMOTime currTime = myFirstTime2Show;
        int pos = 31;// + /*!!!currTime*/ - myFirstTime2Show;
        double glpos = (double) pos / width;
        const double ticSize = 4 / height;
        while (pos < width + 50) {
            const std::string timeStr = (gHumanReadableTime
                                         ? time2string(currTime % 3600000).substr(3) // only write mn:ss
                                         : toString((int)STEPS2TIME(currTime)));
            const double w = 50 / width;
            glTranslated(glpos - w / 2., glh - h20, 0);
            GLHelper::drawText(timeStr, Position(0, 0), 1, fontHeight, RGBColor::WHITE, 0, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE, fontWidth);
            glTranslated(-glpos + w / 2., -glh + h20, 0);

            glBegin(GL_LINES);
            glVertex2d(glpos, glh);
            glVertex2d(glpos, glh - ticSize);
            glEnd();

            const double a = STEPS2TIME(tickDist) * barWidth / STEPS2TIME(myLastTime - myBeginTime);
            pos += (int) a;
            glpos += a / width;
            currTime += tickDist;
        }
    }
}


void
GUITLLogicPhasesTrackerWindow::addValue(std::pair<SUMOTime, MSPhaseDefinition> def) {
    // do not draw while adding
    myLock.lock();
    // set the first time if not set before
    if (myPhases.size() == 0) {
        myBeginTime = def.first;
    }
    // append or set the phase
    if (myPhases.size() == 0 || *(myPhases.end() - 1) != def.second) {
        myPhases.push_back(def.second);
        myDurations.push_back(DELTA_T);
    } else {
        *(myDurations.end() - 1) += DELTA_T;
    }
    // set the last time a phase was added at
    myLastTime = def.first;
    // allow drawing
    myLock.unlock();
}


long
GUITLLogicPhasesTrackerWindow::onConfigure(FXObject* sender, FXSelector sel, void* ptr) {
    myPanel->onConfigure(sender, sel, ptr);
    return FXMainWindow::onConfigure(sender, sel, ptr);
}


long
GUITLLogicPhasesTrackerWindow::onPaint(FXObject* sender, FXSelector sel, void* ptr) {
    myPanel->onPaint(sender, sel, ptr);
    return FXMainWindow::onPaint(sender, sel, ptr);
}


long
GUITLLogicPhasesTrackerWindow::onSimStep(FXObject*, FXSelector, void*) {
    update();
    return 1;
}


void
GUITLLogicPhasesTrackerWindow::setBeginTime(SUMOTime time) {
    myBeginTime = time;
}


/****************************************************************************/
