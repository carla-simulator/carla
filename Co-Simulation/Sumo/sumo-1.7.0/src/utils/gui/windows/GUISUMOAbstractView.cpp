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
/// @file    GUISUMOAbstractView.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Andreas Gaubatz
/// @date    Sept 2002
///
// The base class for a view
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utility>
#include <cmath>
#include <cassert>
#include <limits>
#include <fxkeys.h>
#ifdef HAVE_GL2PS
#include <gl2ps.h>
#endif
#include <utils/foxtools/FXSingleEventThread.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/common/RGBColor.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SysUtils.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/shapes/PointOfInterest.h>
#include <utils/gui/globjects/GUIPointOfInterest.h>
#include <utils/gui/globjects/GUIPolygon.h>
#include <utils/gui/windows/GUIDialog_ViewSettings.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <foreign/fontstash/fontstash.h>

#include "GUISUMOAbstractView.h"
#include "GUIMainWindow.h"
#include "GUIGlChildWindow.h"
#include "GUIDanielPerspectiveChanger.h"
#include "GUIDialog_EditViewport.h"

#ifdef HAVE_GDAL
#if __GNUC__ > 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <gdal_priv.h>
#if __GNUC__ > 3
#pragma GCC diagnostic pop
#endif
#endif


// ===========================================================================
// debug constants
// ===========================================================================
//#define DEBUG_SNAPSHOT

// ===========================================================================
// static members
// ===========================================================================

const double GUISUMOAbstractView::SENSITIVITY = 0.1; // meters


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUISUMOAbstractView - FOX callback mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUISUMOAbstractView) GUISUMOAbstractViewMap[] = {
    FXMAPFUNC(SEL_CONFIGURE,            0,      GUISUMOAbstractView::onConfigure),
    FXMAPFUNC(SEL_PAINT,                0,      GUISUMOAbstractView::onPaint),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS,      0,      GUISUMOAbstractView::onLeftBtnPress),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE,    0,      GUISUMOAbstractView::onLeftBtnRelease),
    FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,    0,      GUISUMOAbstractView::onMiddleBtnPress),
    FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,  0,      GUISUMOAbstractView::onMiddleBtnRelease),
    FXMAPFUNC(SEL_RIGHTBUTTONPRESS,     0,      GUISUMOAbstractView::onRightBtnPress),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   0,      GUISUMOAbstractView::onRightBtnRelease),
    FXMAPFUNC(SEL_DOUBLECLICKED,        0,      GUISUMOAbstractView::onDoubleClicked),
    FXMAPFUNC(SEL_MOUSEWHEEL,           0,      GUISUMOAbstractView::onMouseWheel),
    FXMAPFUNC(SEL_MOTION,               0,      GUISUMOAbstractView::onMouseMove),
    FXMAPFUNC(SEL_LEAVE,                0,      GUISUMOAbstractView::onMouseLeft),
    FXMAPFUNC(SEL_KEYPRESS,             0,      GUISUMOAbstractView::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE,           0,      GUISUMOAbstractView::onKeyRelease),

};


FXIMPLEMENT_ABSTRACT(GUISUMOAbstractView, FXGLCanvas, GUISUMOAbstractViewMap, ARRAYNUMBER(GUISUMOAbstractViewMap))


/* -------------------------------------------------------------------------
 * GUISUMOAbstractView - methods
 * ----------------------------------------------------------------------- */
GUISUMOAbstractView::GUISUMOAbstractView(FXComposite* p, GUIMainWindow& app, GUIGlChildWindow* parent, const SUMORTree& grid, FXGLVisual* glVis, FXGLCanvas* share) :
    FXGLCanvas(p, glVis, share, p, MID_GLCANVAS, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0),
    myApp(&app),
    myParent(parent),
    myGrid(&grid),
    myChanger(nullptr),
    myMouseHotspotX(app.getDefaultCursor()->getHotX()),
    myMouseHotspotY(app.getDefaultCursor()->getHotY()),
    myPopup(nullptr),
    myPopupPosition(Position(0, 0)),
    myUseToolTips(false),
    myAmInitialised(false),
    myViewportChooser(nullptr),
    myWindowCursorPositionX(getWidth() / 2),
    myWindowCursorPositionY(getHeight() / 2),
    myVisualizationChanger(nullptr),
    myFrameDrawTime(0) {
    setTarget(this);
    enable();
    flags |= FLAG_ENABLED;
    myInEditMode = false;
    // show the middle at the beginning
    myChanger = new GUIDanielPerspectiveChanger(*this, *myGrid);
    myVisualizationSettings = &gSchemeStorage.getDefault();
    myVisualizationSettings->gaming = myApp->isGaming();
    gSchemeStorage.setViewport(this);
}


GUISUMOAbstractView::~GUISUMOAbstractView() {
    gSchemeStorage.setDefault(myVisualizationSettings->name);
    gSchemeStorage.saveViewport(myChanger->getXPos(), myChanger->getYPos(), myChanger->getZPos(), myChanger->getRotation());
    delete myPopup;
    delete myChanger;
    delete myViewportChooser;
    delete myVisualizationChanger;
    // cleanup decals
    for (std::vector<GUISUMOAbstractView::Decal>::iterator it = myDecals.begin(); it != myDecals.end(); ++it) {
        delete it->image;
    }
    for (auto i : myAdditionallyDrawn) {
        i.first->removeActiveAddVisualisation(this, ~0); // remove all
    }
}


bool
GUISUMOAbstractView::isInEditMode() {
    return myInEditMode;
}


GUIPerspectiveChanger&
GUISUMOAbstractView::getChanger() const {
    return *myChanger;
}


void
GUISUMOAbstractView::updateToolTip() {
    if (!myUseToolTips) {
        return;
    }
    update();
}


Position
GUISUMOAbstractView::getPositionInformation() const {
    return screenPos2NetPos(myWindowCursorPositionX, myWindowCursorPositionY);
}


Position
GUISUMOAbstractView::snapToActiveGrid(const Position& pos, bool snapXY) const {
    Position result = pos;
    if (myVisualizationSettings->showGrid) {
        if (snapXY) {
            const double xRest = std::fmod(pos.x(), myVisualizationSettings->gridXSize) + (pos.x() < 0 ? myVisualizationSettings->gridXSize : 0);
            const double yRest = std::fmod(pos.y(), myVisualizationSettings->gridYSize) + (pos.y() < 0 ? myVisualizationSettings->gridYSize : 0);
            result.setx(pos.x() - xRest + (xRest < myVisualizationSettings->gridXSize * 0.5 ? 0 : myVisualizationSettings->gridXSize));
            result.sety(pos.y() - yRest + (yRest < myVisualizationSettings->gridYSize * 0.5 ? 0 : myVisualizationSettings->gridYSize));
        } else {
            // snapZToActiveGrid uses grid Y Size
            const double zRest = std::fmod(pos.z(), myVisualizationSettings->gridYSize) + (pos.z() < 0 ? myVisualizationSettings->gridYSize : 0);
            result.setz(pos.z() - zRest + (zRest < myVisualizationSettings->gridYSize * 0.5 ? 0 : myVisualizationSettings->gridYSize));
        }
    }
    return result;
}


Position
GUISUMOAbstractView::screenPos2NetPos(int x, int y) const {
    Boundary bound = myChanger->getViewport();
    double xNet = bound.xmin() + bound.getWidth() * x / getWidth();
    // cursor origin is in the top-left corner
    double yNet = bound.ymin() + bound.getHeight() * (getHeight() - y) / getHeight();
    // rotate around the viewport center
    if (myChanger->getRotation() != 0) {
        return Position(xNet, yNet).rotateAround2D(-DEG2RAD(myChanger->getRotation()), bound.getCenter());
    } else {
        return Position(xNet, yNet);
    }
}


void
GUISUMOAbstractView::addDecals(const std::vector<Decal>& decals) {
    myDecals.insert(myDecals.end(), decals.begin(), decals.end());
}


void
GUISUMOAbstractView::updatePositionInformation() const {
    Position pos = getPositionInformation();
    std::string text = "x:" + toString(pos.x()) + ", y:" + toString(pos.y());
    myApp->getCartesianLabel().setText(text.c_str());
    GeoConvHelper::getFinal().cartesian2geo(pos);
    if (GeoConvHelper::getFinal().usingGeoProjection()) {
        text = "lat:" + toString(pos.y(), gPrecisionGeo) + ", lon:" + toString(pos.x(), gPrecisionGeo);
    } else {
        text = "x:" + toString(pos.x()) + ", y:" + toString(pos.y());
    }
    myApp->getGeoLabel().setText(text.c_str());
}


int
GUISUMOAbstractView::doPaintGL(int /*mode*/, const Boundary& /*boundary*/) {
    return 0;
}


void
GUISUMOAbstractView::doInit() {
}


Boundary
GUISUMOAbstractView::getVisibleBoundary() const {
    return myChanger->getViewport();
}


void
GUISUMOAbstractView::paintGL() {
    if (getWidth() == 0 || getHeight() == 0) {
        return;
    }
    const long start = SysUtils::getCurrentMillis();

    if (getTrackedID() != GUIGlObject::INVALID_ID) {
        centerTo(getTrackedID(), false);
    }

    GUIGlID id = GUIGlObject::INVALID_ID;
    if (myUseToolTips) {
        id = getObjectUnderCursor();
    }

    // draw
    glClearColor(
        myVisualizationSettings->backgroundColor.red() / 255.f,
        myVisualizationSettings->backgroundColor.green() / 255.f,
        myVisualizationSettings->backgroundColor.blue() / 255.f,
        myVisualizationSettings->backgroundColor.alpha() / 255.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (myVisualizationSettings->dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
    glEnable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    Boundary bound = applyGLTransform();
    doPaintGL(GL_RENDER, bound);
    displayLegends();
    const long end = SysUtils::getCurrentMillis();
    myFrameDrawTime = end - start;
    if (myVisualizationSettings->fps) {
        drawFPS();
    }
    // check whether the select mode /tooltips)
    //  shall be computed, too
    if (myUseToolTips && id != GUIGlObject::INVALID_ID) {
        showToolTipFor(id);
    }
    swapBuffers();
}


GUIGlID
GUISUMOAbstractView::getObjectUnderCursor() {
    return getObjectAtPosition(getPositionInformation());
}


std::vector<GUIGlID>
GUISUMOAbstractView::getObjectsUnderCursor() {
    return getObjectsAtPosition(getPositionInformation(), SENSITIVITY);
}


std::vector<GUIGlObject*>
GUISUMOAbstractView::getGUIGlObjectsUnderCursor() {
    return getGUIGlObjectsAtPosition(getPositionInformation(), SENSITIVITY);
}


std::vector<GUIGlObject*>
GUISUMOAbstractView::getGUIGlObjectsUnderSnappedCursor() {
    return getGUIGlObjectsAtPosition(snapToActiveGrid(getPositionInformation()), SENSITIVITY);
}


GUIGlID
GUISUMOAbstractView::getObjectAtPosition(Position pos) {
    // calculate a boundary for the given position
    Boundary positionBoundary;
    positionBoundary.add(pos);
    positionBoundary.grow(SENSITIVITY);
    const std::vector<GUIGlID> ids = getObjectsInBoundary(positionBoundary, true);
    // Interpret results
    int idMax = 0;
    double maxLayer = -std::numeric_limits<double>::max();
    // iterate over obtained GUIGlIDs
    for (const auto& i : ids) {
        // obtain GUIGlObject
        GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(i);
        // check that GUIGlObject exist
        if (o == nullptr) {
            continue;
        }
        // check that GUIGlObject isn't the network
        if (o->getGlID() == 0) {
            continue;
        }
        //std::cout << "point selection hit " << o->getMicrosimID() << "\n";
        GUIGlObjectType type = o->getType();
        // avoid network
        if (type != GLO_NETWORK) {
            double layer = (double)type;
            // determine an "abstract" layer for shapes
            //  this "layer" resembles the layer of the shape
            //  taking into account the stac of other objects
            if (type == GLO_POI || type == GLO_POLYGON) {
                layer = dynamic_cast<Shape*>(o)->getShapeLayer();
            }
            if (type == GLO_LANE && GUIVisualizationSettings::UseMesoSim) {
                // do not select lanes in meso mode
                continue;
            }
            // check whether the current object is above a previous one
            if (layer > maxLayer) {
                idMax = i;
                maxLayer = layer;
            }
        }
        // unblock object
        GUIGlObjectStorage::gIDStorage.unblockObject(i);
    }
    return idMax;
}


std::vector<GUIGlID>
GUISUMOAbstractView::getObjectsAtPosition(Position pos, double radius) {
    // declare result vector
    std::vector<GUIGlID> result;
    // calculate boundary
    Boundary selection;
    selection.add(pos);
    selection.grow(radius);
    // obtain GUIGlID of objects in boundary
    const std::vector<GUIGlID> ids = getObjectsInBoundary(selection, true);
    // iterate over obtained GUIGlIDs
    for (const auto& i : ids) {
        // obtain GUIGlObject
        GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(i);
        // check that GUIGlObject exist
        if (o == nullptr) {
            continue;
        }
        // check that GUIGlObject isn't the network
        if (o->getGlID() == 0) {
            continue;
        }
        //std::cout << "point selection hit " << o->getMicrosimID() << "\n";
        GUIGlObjectType type = o->getType();
        // avoid network
        if (type != GLO_NETWORK) {
            result.push_back(i);
        }
        // unblock object
        GUIGlObjectStorage::gIDStorage.unblockObject(i);
    }
    return result;
}


std::vector<GUIGlObject*>
GUISUMOAbstractView::getGUIGlObjectsAtPosition(Position pos, double radius) {
    // declare result vector
    std::vector<GUIGlObject*> result;
    // calculate boundary
    Boundary selection;
    selection.add(pos);
    selection.grow(radius);
    // obtain GUIGlID of objects in boundary
    const std::vector<GUIGlID> ids = getObjectsInBoundary(selection, true);
    // iterate over obtained GUIGlIDs
    for (const auto& i : ids) {
        // obtain GUIGlObject
        GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(i);
        // check that GUIGlObject exist
        if (o == nullptr) {
            continue;
        }
        // check that GUIGlObject isn't the network
        if (o->getGlID() == 0) {
            continue;
        }
        result.push_back(o);
        // unblock object
        GUIGlObjectStorage::gIDStorage.unblockObject(i);
    }
    return result;
}


std::vector<GUIGlID>
GUISUMOAbstractView::getObjectsInBoundary(Boundary bound, bool singlePosition) {
    const int NB_HITS_MAX = 1024 * 1024;
    // Prepare the selection mode
    static GUIGlID hits[NB_HITS_MAX];
    static GLint nb_hits = 0;
    glSelectBuffer(NB_HITS_MAX, hits);
    glInitNames();

    myVisualizationSettings->scale = m2p(SUMO_const_laneWidth);
    Boundary oldViewPort = myChanger->getViewport(false); // backup the actual viewPort
    myChanger->setViewport(bound);
    bound = applyGLTransform(false);
    // enable draw for selecting (to draw objects with less details)
    if (singlePosition) {
        myVisualizationSettings->drawForPositionSelection = true;
    } else {
        myVisualizationSettings->drawForRectangleSelection = true;
    }
    int hits2 = doPaintGL(GL_SELECT, bound);
    // reset flags
    myVisualizationSettings->drawForPositionSelection = false;
    myVisualizationSettings->drawForRectangleSelection = false;
    // Get the results
    nb_hits = glRenderMode(GL_RENDER);
    if (nb_hits == -1) {
        myApp->setStatusBarText("Selection in boundary failed. Try to select fewer than " + toString(hits2) + " items");
    }
    std::vector<GUIGlID> result;
    GLuint numNames;
    GLuint* ptr = hits;
    for (int i = 0; i < nb_hits; ++i) {
        numNames = *ptr;
        ptr += 3;
        for (int j = 0; j < (int)numNames; j++) {
            result.push_back(*ptr);
            ptr++;
        }
    }
    // switch viewport back to normal
    myChanger->setViewport(oldViewPort);
    return result;
}


void
GUISUMOAbstractView::showToolTipFor(const GUIGlID id) {
    if (id != 0) {
        GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        if (object != nullptr) {
            Position pos = getPositionInformation();
            pos.add(0, p2m(15));
            std::string label = object->getFullName();
            if (myVisualizationSettings->edgeValue.show &&
                    (object->getType() == GLO_EDGE || object->getType() == GLO_LANE)) {
                const int activeScheme = myVisualizationSettings->getLaneEdgeMode();
                label += " (" + toString(object->getColorValue(*myVisualizationSettings, activeScheme)) + ")";
            }
            GLHelper::drawTextBox(label, pos, GLO_MAX - 1, p2m(20), RGBColor::BLACK, RGBColor(255, 179, 0, 255));
            GUIGlObjectStorage::gIDStorage.unblockObject(id);
        }
    }
}


void
GUISUMOAbstractView::paintGLGrid() {
    // obtain minimum grid
    double minimumSizeGrid = (myVisualizationSettings->gridXSize < myVisualizationSettings->gridYSize) ? myVisualizationSettings->gridXSize : myVisualizationSettings->gridYSize;
    // Check if the distance is enought to draw grid
    if (myVisualizationSettings->scale * myVisualizationSettings->addSize.getExaggeration(*myVisualizationSettings, nullptr) >= (25 / minimumSizeGrid)) {
        glEnable(GL_DEPTH_TEST);
        glLineWidth(1);
        // get multiplication values (2 is the marging)
        int multXmin = (int)(myChanger->getViewport().xmin() / myVisualizationSettings->gridXSize) - 2;
        int multYmin = (int)(myChanger->getViewport().ymin() / myVisualizationSettings->gridYSize) - 2;
        int multXmax = (int)(myChanger->getViewport().xmax() / myVisualizationSettings->gridXSize) + 2;
        int multYmax = (int)(myChanger->getViewport().ymax() / myVisualizationSettings->gridYSize) + 2;
        // obtain references
        double xmin = myVisualizationSettings->gridXSize * multXmin;
        double ymin = myVisualizationSettings->gridYSize * multYmin;
        double xmax = myVisualizationSettings->gridXSize * multXmax;
        double ymax = myVisualizationSettings->gridYSize * multYmax;
        double xpos = xmin;
        double ypos = ymin;
        // move drawing matrix
        glTranslated(0, 0, .55);
        glColor3d(0.5, 0.5, 0.5);
        // draw horizontal lines
        glBegin(GL_LINES);
        while (ypos <= ymax) {
            glVertex2d(xmin, ypos);
            glVertex2d(xmax, ypos);
            ypos += myVisualizationSettings->gridYSize;
        }
        // draw vertical lines
        while (xpos <= xmax) {
            glVertex2d(xpos, ymin);
            glVertex2d(xpos, ymax);
            xpos += myVisualizationSettings->gridXSize;
        }
        glEnd();
        glTranslated(0, 0, -.55);
    }
}


void
GUISUMOAbstractView::displayLegend() {
    // compute the scale bar length
    int length = 1;
    const std::string text("10000000000");
    int noDigits = 1;
    int pixelSize = (int) m2p((double) length);
    while (pixelSize <= 20) {
        length *= 10;
        noDigits++;
        if (noDigits > (int)text.length()) {
            return;
        }
        pixelSize = (int) m2p((double) length);
    }
    glLineWidth(1.0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // draw the scale bar
    const double z = -1;
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPushMatrix();
    glTranslated(0, 0, z);

    double len = (double) pixelSize / (double)(getWidth() - 1) * (double) 2.0;
    glColor3d(0, 0, 0);
    double o = double(15) / double(getHeight());
    double o2 = o + o;
    double oo = double(5) / double(getHeight());
    glBegin(GL_LINES);
    // vertical
    glVertex2d(-.98, -1. + o);
    glVertex2d(-.98 + len, -1. + o);
    // tick at begin
    glVertex2d(-.98, -1. + o);
    glVertex2d(-.98, -1. + o2);
    // tick at end
    glVertex2d(-.98 + len, -1. + o);
    glVertex2d(-.98 + len, -1. + o2);
    glEnd();
    glPopMatrix();

    const double fontHeight = 0.1 * 300. / getHeight();
    const double fontWidth = 0.1 * 300. / getWidth();
    // draw 0
    GLHelper::drawText("0", Position(-.99, -0.99 + o2 + oo), z, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT, fontWidth);

    // draw current scale
    GLHelper::drawText((text.substr(0, noDigits) + "m").c_str(), Position(-.99 + len, -0.99 + o2 + oo), z, fontHeight, RGBColor::BLACK, 0, FONS_ALIGN_LEFT, fontWidth);

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void
GUISUMOAbstractView::displayLegends() {
    if (myVisualizationSettings->showSizeLegend) {
        displayLegend();
    }
    if (myVisualizationSettings->showColorLegend) {
        displayColorLegend(myVisualizationSettings->getLaneEdgeScheme(), false);
    }
    if (myVisualizationSettings->showVehicleColorLegend) {
        displayColorLegend(myVisualizationSettings->vehicleColorer.getScheme(), true);
    }
}

void
GUISUMOAbstractView::displayColorLegend(const GUIColorScheme& scheme, bool leftSide) {
    // compute the scale bar length
    glLineWidth(1.0);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    const double z = -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glPushMatrix();
    glTranslated(0, 0, z);

    const bool fixed = scheme.isFixed();
    const int numColors = (int)scheme.getColors().size();

    // vertical
    double right = 0.98;
    double left = 0.95;
    double textX = left - 0.01;
    FONSalign textAlign = FONS_ALIGN_RIGHT;
    const double top = -0.8;
    const double bot = 0.8;
    const double dy = (top - bot) / numColors;
    const double bot2 = fixed ? bot : bot + dy / 2;
    // legend placement
    if (leftSide) {
        right = -right;
        left = -left;
        std::swap(right, left);
        textX = right + 0.01;
        textAlign = FONS_ALIGN_LEFT;
    }
    // draw black boundary around legend colors
    glColor3d(0, 0, 0);
    glBegin(GL_LINES);
    glVertex2d(right, top);
    glVertex2d(right, bot2);
    glVertex2d(left, bot2);
    glVertex2d(left, top);
    glVertex2d(right, top);
    glVertex2d(left, top);
    glVertex2d(right, bot2);
    glVertex2d(left, bot2);
    glEnd();

    const double fontHeight = 0.20 * 300. / getHeight();
    const double fontWidth = 0.20 * 300. / getWidth();

    const int fadeSteps = fixed ? 1 : 10;
    double colorStep = dy / fadeSteps;
    for (int i = 0; i < numColors; i++) {
        RGBColor col = scheme.getColors()[i];
        const double topi = top - i * dy;
        //const double boti = top - (i + 1) * dy;
        //std::cout << " col=" << scheme.getColors()[i] << " i=" << i << " topi=" << topi << " boti=" << boti << "\n";
        if (i + 1 < numColors) {
            // fade
            RGBColor col2 = scheme.getColors()[i + 1];
            for (double j = 0.0; j < fadeSteps; j++) {
                GLHelper::setColor(RGBColor::interpolate(col, col2, j / fadeSteps));
                glBegin(GL_QUADS);
                glVertex2d(left, topi - j * colorStep);
                glVertex2d(right, topi - j * colorStep);
                glVertex2d(right, topi - (j + 1) * colorStep);
                glVertex2d(left, topi - (j + 1) * colorStep);
                glEnd();
            }
        } else {
            GLHelper::setColor(col);
            glBegin(GL_QUADS);
            glVertex2d(left, topi);
            glVertex2d(right, topi);
            glVertex2d(right, bot2);
            glVertex2d(left, bot2);
            glEnd();
        }

        const double threshold = scheme.getThresholds()[i];
        std::string name = scheme.getNames()[i];
        std::string text = fixed || threshold == GUIVisualizationSettings::MISSING_DATA ? name : toString(threshold);

        const double bgShift = 0.0;
        const double textShift = 0.02;

        GLHelper::setColor(RGBColor::WHITE);
        glTranslated(0, 0, 0.1);
        glBegin(GL_QUADS);
        glVertex2d(left, topi + fontHeight * bgShift);
        glVertex2d(left - fontWidth * text.size() / 2, topi + fontHeight * bgShift);
        glVertex2d(left - fontWidth * text.size() / 2, topi + fontHeight * (1 + bgShift));
        glVertex2d(left, topi + fontHeight * (1 + bgShift));
        glEnd();
        glTranslated(0, 0, -0.1);
        GLHelper::drawText(text, Position(textX, topi + textShift), 0, fontHeight, RGBColor::BLACK, 0, textAlign, fontWidth);
    }
    glPopMatrix();
    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


double
GUISUMOAbstractView::getFPS() const {
    return 1000.0 / MAX2((long)1, myFrameDrawTime);
}

void
GUISUMOAbstractView::drawFPS() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    const double fontHeight = 0.2 * 300. / getHeight();
    const double fontWidth = 0.2 * 300. / getWidth();
    GLHelper::drawText(toString((int)getFPS()) + " FPS", Position(0.82, 0.88), -1, fontHeight, RGBColor::RED, 0, FONS_ALIGN_LEFT, fontWidth);

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


double
GUISUMOAbstractView::m2p(double meter) const {
    return  meter * getWidth() / myChanger->getViewport().getWidth();
}


double
GUISUMOAbstractView::p2m(double pixel) const {
    return pixel * myChanger->getViewport().getWidth() / getWidth();
}


void
GUISUMOAbstractView::recenterView() {
    myChanger->setViewport(*myGrid);
}


void
GUISUMOAbstractView::centerTo(GUIGlID id, bool applyZoom, double zoomDist) {
    GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
    if (o != nullptr && dynamic_cast<GUIGlObject*>(o) != nullptr) {
        if (applyZoom && zoomDist < 0) {
            myChanger->setViewport(o->getCenteringBoundary());
            update(); // only update when centering onto an object once
        } else {
            // called during tracking. update is triggered somewhere else
            myChanger->centerTo(o->getCenteringBoundary().getCenter(), zoomDist, applyZoom);
            updatePositionInformation();
        }
    }
    GUIGlObjectStorage::gIDStorage.unblockObject(id);
}


void
GUISUMOAbstractView::centerTo(const Position& pos, bool applyZoom, double zoomDist) {
    // called during tracking. update is triggered somewhere else
    myChanger->centerTo(pos, zoomDist, applyZoom);
    updatePositionInformation();
}


void
GUISUMOAbstractView::centerTo(const Boundary& bound) {
    myChanger->setViewport(bound);
    update();
}

/*
bool
GUISUMOAbstractView::allowRotation() const
{
    return myParent->allowRotation();
}
*/

Position
GUISUMOAbstractView::getWindowCursorPosition() const {
    return Position(myWindowCursorPositionX, myWindowCursorPositionY);
}


void
GUISUMOAbstractView::setWindowCursorPosition(FXint x, FXint y) {
    myWindowCursorPositionX = x + myMouseHotspotX;
    myWindowCursorPositionY = y + myMouseHotspotY;
}


FXbool
GUISUMOAbstractView::makeCurrent() {
    FXbool ret = FXGLCanvas::makeCurrent();
    return ret;
}


long
GUISUMOAbstractView::onConfigure(FXObject*, FXSelector, void*) {
    if (makeCurrent()) {
        glViewport(0, 0, getWidth() - 1, getHeight() - 1);
        glClearColor(
            myVisualizationSettings->backgroundColor.red() / 255.f,
            myVisualizationSettings->backgroundColor.green() / 255.f,
            myVisualizationSettings->backgroundColor.blue() / 255.f,
            myVisualizationSettings->backgroundColor.alpha() / 255.f);
        doInit();
        myAmInitialised = true;
        makeNonCurrent();
        checkSnapshots();
    }
    return 1;
}


long
GUISUMOAbstractView::onPaint(FXObject*, FXSelector, void*) {
    if (!isEnabled() || !myAmInitialised) {
        return 1;
    }
    if (makeCurrent()) {
        paintGL();
        makeNonCurrent();
    }
    return 1;
}


const Position&
GUISUMOAbstractView::getPopupPosition() const {
    return myPopupPosition;
}

void
GUISUMOAbstractView::destroyPopup() {
    if (myPopup != nullptr) {
        delete myPopup;
        myPopupPosition.set(0, 0);
        myPopup = nullptr;
    }
}


long
GUISUMOAbstractView::onLeftBtnPress(FXObject*, FXSelector, void* data) {
    destroyPopup();
    setFocus();
    FXEvent* e = (FXEvent*) data;
    // check whether the selection-mode is activated
    if ((e->state & CONTROLMASK) != 0) {
        // toggle selection of object under cursor
        if (makeCurrent()) {
            int id = getObjectUnderCursor();
            if (id != 0) {
                gSelected.toggleSelection(id);
            }
            makeNonCurrent();
            if (id != 0) {
                // possibly, the selection-colouring is used,
                //  so we should update the screen again...
                update();
            }
        }
    }
    if ((e->state & SHIFTMASK) != 0) {
        // track vehicle or person under cursor
        if (makeCurrent()) {
            int id = getObjectUnderCursor();
            if (id != 0) {
                GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
                if (o != nullptr) {
                    if (o->getType() == GLO_VEHICLE || o->getType() == GLO_PERSON) {
                        startTrack(id);
                    } else if (o->getType() == GLO_REROUTER_EDGE) {
                        o->onLeftBtnPress(data);
                        update();
                    }
                }
            }
            makeNonCurrent();
        }
    }
    myChanger->onLeftBtnPress(data);
    grab();
    // Check there are double click
    if (e->click_count == 2) {
        handle(this, FXSEL(SEL_DOUBLECLICKED, 0), data);
    }
    return 1;
}


long
GUISUMOAbstractView::onLeftBtnRelease(FXObject*, FXSelector, void* data) {
    destroyPopup();
    myChanger->onLeftBtnRelease(data);
    if (myApp->isGaming()) {
        onGamingClick(getPositionInformation());
    }
    ungrab();
    return 1;
}


long
GUISUMOAbstractView::onMiddleBtnPress(FXObject*, FXSelector, void*) {
    return 1;
}


long
GUISUMOAbstractView::onMiddleBtnRelease(FXObject*, FXSelector, void*) {
    return 1;
}


long
GUISUMOAbstractView::onRightBtnPress(FXObject*, FXSelector, void* data) {
    destroyPopup();
    myChanger->onRightBtnPress(data);
    grab();
    return 1;
}


long
GUISUMOAbstractView::onRightBtnRelease(FXObject* o, FXSelector sel, void* data) {
    destroyPopup();
    onMouseMove(o, sel, data);
    if (!myChanger->onRightBtnRelease(data) && !myApp->isGaming()) {
        openObjectDialog();
    }
    if (myApp->isGaming()) {
        onGamingRightClick(getPositionInformation());
    }
    ungrab();
    return 1;
}


long
GUISUMOAbstractView::onDoubleClicked(FXObject*, FXSelector, void*) {
    return 1;
}


long
GUISUMOAbstractView::onMouseWheel(FXObject*, FXSelector, void* data) {
    if (!myApp->isGaming()) {
        myChanger->onMouseWheel(data);
        // upddate viewport
        if (myViewportChooser != nullptr) {
            myViewportChooser->setValues(myChanger->getZoom(),
                                         myChanger->getXPos(), myChanger->getYPos(),
                                         myChanger->getRotation());
        }
        updatePositionInformation();
    }
    return 1;
}


long
GUISUMOAbstractView::onMouseMove(FXObject*, FXSelector, void* data) {
    // if popup exist but isn't shown, destroy it first
    if (myPopup && (myPopup->shown() == false)) {
        destroyPopup();
    }
    if (myPopup == nullptr) {
        if (myViewportChooser == nullptr || !myViewportChooser->haveGrabbed()) {
            myChanger->onMouseMove(data);
        }
        if (myViewportChooser != nullptr) {
            myViewportChooser->setValues(myChanger->getZoom(),
                                         myChanger->getXPos(), myChanger->getYPos(),
                                         myChanger->getRotation());
        }
        updatePositionInformation();
    }
    return 1;
}


long
GUISUMOAbstractView::onMouseLeft(FXObject*, FXSelector, void* /*data*/) {
    return 1;
}


void
GUISUMOAbstractView::openObjectDialog() {
    ungrab();
    if (!isEnabled() || !myAmInitialised) {
        return;
    }
    if (makeCurrent()) {
        // initialise the select mode
        int id = getObjectUnderCursor();
        GUIGlObject* o = nullptr;
        if (id != 0) {
            o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        } else {
            o = GUIGlObjectStorage::gIDStorage.getNetObject();
        }
        if (o != nullptr) {
            myPopup = o->getPopUpMenu(*myApp, *this);
            int x, y;
            FXuint b;
            myApp->getCursorPosition(x, y, b);
            myPopup->setX(x + myApp->getX());
            myPopup->setY(y + myApp->getY());
            myPopup->create();
            myPopup->show();
            myPopupPosition = getPositionInformation();
            myChanger->onRightBtnRelease(nullptr);
            GUIGlObjectStorage::gIDStorage.unblockObject(id);
            setFocus();
        }
        makeNonCurrent();
    }
}


long
GUISUMOAbstractView::onKeyPress(FXObject* o, FXSelector sel, void* data) {
    if (myPopup != nullptr) {
        return myPopup->onKeyPress(o, sel, data);
    } else {
        FXEvent* e = (FXEvent*) data;
        if (e->state & CONTROLMASK) {
            if (e->code == FX::KEY_Page_Up) {
                myVisualizationSettings->gridXSize *= 2;
                myVisualizationSettings->gridYSize *= 2;
                update();
                return 1;
            } else if (e->code == FX::KEY_Page_Down) {
                myVisualizationSettings->gridXSize /= 2;
                myVisualizationSettings->gridYSize /= 2;
                update();
                return 1;
            }
        }
        FXGLCanvas::onKeyPress(o, sel, data);
        return myChanger->onKeyPress(data);
    }
}


long
GUISUMOAbstractView::onKeyRelease(FXObject* o, FXSelector sel, void* data) {
    if (myPopup != nullptr) {
        return myPopup->onKeyRelease(o, sel, data);
    } else {
        FXGLCanvas::onKeyRelease(o, sel, data);
        return myChanger->onKeyRelease(data);
    }
}


// ------------ Dealing with snapshots
void
GUISUMOAbstractView::addSnapshot(SUMOTime time, const std::string& file, const int w, const int h) {
#ifdef DEBUG_SNAPSHOT
    std::cout << "add snapshot time=" << time << " file=" << file << "\n";
#endif
    FXMutexLock lock(mySnapshotsMutex);
    mySnapshots[time].push_back(std::make_tuple(file, w, h));
}


std::string
GUISUMOAbstractView::makeSnapshot(const std::string& destFile, const int w, const int h) {
    if (w >= 0) {
        resize(w, h);
        repaint();
    }
    std::string errorMessage;
    FXString ext = FXPath::extension(destFile.c_str());
    const bool useGL2PS = ext == "ps" || ext == "eps" || ext == "pdf" || ext == "svg" || ext == "tex" || ext == "pgf";
#ifdef HAVE_FFMPEG
    const bool useVideo = destFile == "" || ext == "h264" || ext == "hevc";
#endif
    for (int i = 0; i < 10 && !makeCurrent(); ++i) {
        FXSingleEventThread::sleep(100);
    }
    // draw
    glClearColor(
        myVisualizationSettings->backgroundColor.red() / 255.f,
        myVisualizationSettings->backgroundColor.green() / 255.f,
        myVisualizationSettings->backgroundColor.blue() / 255.f,
        myVisualizationSettings->backgroundColor.alpha() / 255.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (myVisualizationSettings->dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
    glEnable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    applyGLTransform();

    if (useGL2PS) {
#ifdef HAVE_GL2PS
        GLint format = GL2PS_PS;
        if (ext == "ps") {
            format = GL2PS_PS;
        } else if (ext == "eps") {
            format = GL2PS_EPS;
        } else if (ext == "pdf") {
            format = GL2PS_PDF;
        } else if (ext == "tex") {
            format = GL2PS_TEX;
        } else if (ext == "svg") {
            format = GL2PS_SVG;
        } else if (ext == "pgf") {
            format = GL2PS_PGF;
        } else {
            return "Could not save '" + destFile + "'.\n Unrecognized format '" + std::string(ext.text()) + "'.";
        }
        FILE* fp = fopen(destFile.c_str(), "wb");
        if (fp == 0) {
            return "Could not save '" + destFile + "'.\n Could not open file for writing";
        }
        GLHelper::setGL2PS();
        GLint buffsize = 0, state = GL2PS_OVERFLOW;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        while (state == GL2PS_OVERFLOW) {
            buffsize += 1024 * 1024;
            gl2psBeginPage(destFile.c_str(), "sumo-gui; https://sumo.dlr.de", viewport, format, GL2PS_SIMPLE_SORT,
                           GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,
                           GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, "out.eps");
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            // draw decals (if not in grabbing mode)
            if (!myUseToolTips) {
                drawDecals();
                if (myVisualizationSettings->showGrid) {
                    paintGLGrid();
                }
            }
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            Boundary viewPort = myChanger->getViewport();
            const float minB[2] = { (float)viewPort.xmin(), (float)viewPort.ymin() };
            const float maxB[2] = { (float)viewPort.xmax(), (float)viewPort.ymax() };
            myVisualizationSettings->scale = m2p(SUMO_const_laneWidth);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_LINE);
            myGrid->Search(minB, maxB, *myVisualizationSettings);

            displayLegends();
            state = gl2psEndPage();
            glFinish();
        }
        GLHelper::setGL2PS(false);
        fclose(fp);
#else
        return "Could not save '" + destFile + "', gl2ps was not enabled at compile time.";
#endif
    } else {
        doPaintGL(GL_RENDER, myChanger->getViewport());
        displayLegends();
        swapBuffers();
        glFinish();
        FXColor* buf;
        FXMALLOC(&buf, FXColor, getWidth()*getHeight());
        // read from the back buffer
        glReadBuffer(GL_BACK);
        // Read the pixels
        glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buf);
        makeNonCurrent();
        update();
        // mirror
        int mwidth = getWidth();
        int mheight = getHeight();
        FXColor* paa = buf;
        FXColor* pbb = buf + mwidth * (mheight - 1);
        do {
            FXColor* pa = paa;
            paa += mwidth;
            FXColor* pb = pbb;
            pbb -= mwidth;
            do {
                FXColor t = *pa;
                *pa++ = *pb;
                *pb++ = t;
            } while (pa < paa);
        } while (paa < pbb);
        try {
#ifdef HAVE_FFMPEG
            if (useVideo) {
                try {
                    saveFrame(destFile, buf);
                    errorMessage = "video";
                } catch (std::runtime_error& err) {
                    errorMessage = err.what();
                }
            } else
#endif
                if (!MFXImageHelper::saveImage(destFile, getWidth(), getHeight(), buf)) {
                    errorMessage = "Could not save '" + destFile + "'.";
                }
        } catch (InvalidArgument& e) {
            errorMessage = "Could not save '" + destFile + "'.\n" + e.what();
        }
        FXFREE(&buf);
    }
    return errorMessage;
}


void
GUISUMOAbstractView::saveFrame(const std::string& destFile, FXColor* buf) {
    UNUSED_PARAMETER(destFile);
    UNUSED_PARAMETER(buf);
}


void
GUISUMOAbstractView::checkSnapshots() {
    const SUMOTime time = getCurrentTimeStep() - DELTA_T;
#ifdef DEBUG_SNAPSHOT
    std::cout << "check snapshots time=" << time << " registeredTimes=" << mySnapshots.size() << "\n";
#endif
    FXMutexLock lock(mySnapshotsMutex);
    const auto snapIt = mySnapshots.find(time);
    if (snapIt == mySnapshots.end()) {
        return;
    }
    std::vector<std::tuple<std::string, int, int> > files = snapIt->second;
    lock.unlock();
    // decouple map access and painting to avoid deadlock
    for (const auto& entry : files) {
#ifdef DEBUG_SNAPSHOT
        std::cout << "make snapshot time=" << time << " file=" << file << "\n";
#endif
        const std::string& error = makeSnapshot(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry));
        if (error != "" && error != "video") {
            WRITE_WARNING(error);
        }
    }
    // synchronization with a waiting run thread
    lock.lock();
    mySnapshots.erase(time);
    mySnapshotCondition.signal();
#ifdef DEBUG_SNAPSHOT
    std::cout << "  files=" << toString(files) << " myApplicationSnapshots=" << joinToString(*myApplicationSnapshots, ",") << "\n";
#endif
}


void
GUISUMOAbstractView::waitForSnapshots(const SUMOTime snapshotTime) {
    FXMutexLock lock(mySnapshotsMutex);
    if (mySnapshots.count(snapshotTime) > 0) {
        mySnapshotCondition.wait(mySnapshotsMutex);
    }
}


SUMOTime
GUISUMOAbstractView::getCurrentTimeStep() const {
    return 0;
}


void
GUISUMOAbstractView::showViewschemeEditor() {
    if (myVisualizationChanger == nullptr) {
        myVisualizationChanger =
            new GUIDialog_ViewSettings(
            this, myVisualizationSettings,
            &myDecals, &myDecalsLock);
        myVisualizationChanger->create();
    } else {
        myVisualizationChanger->setCurrent(myVisualizationSettings);
    }
    myVisualizationChanger->show();
}


GUIDialog_EditViewport*
GUISUMOAbstractView::getViewportEditor() {
    if (myViewportChooser == nullptr) {
        const FXint minSize = 100;
        const FXint minTitlebarHeight = 20;
        int x = MAX2(0, MIN2(getApp()->reg().readIntEntry(
                                 "VIEWPORT_DIALOG_SETTINGS", "x", 150),
                             getApp()->getRootWindow()->getWidth() - minSize));
        int y = MAX2(minTitlebarHeight, MIN2(getApp()->reg().readIntEntry(
                "VIEWPORT_DIALOG_SETTINGS", "y", 150),
                                             getApp()->getRootWindow()->getHeight() - minSize));
        myViewportChooser = new GUIDialog_EditViewport(this, "Edit Viewport", x, y);
        myViewportChooser->create();
    }
    myViewportChooser->setValues(myChanger->getZoom(),
                                 myChanger->getXPos(), myChanger->getYPos(),
                                 myChanger->getRotation());
    return myViewportChooser;
}


void
GUISUMOAbstractView::showViewportEditor() {
    getViewportEditor(); // make sure it exists;
    Position p(myChanger->getXPos(), myChanger->getYPos(), myChanger->getZPos());
    myViewportChooser->setOldValues(p, Position::INVALID, myChanger->getRotation());
    myViewportChooser->show();
}


void
GUISUMOAbstractView::setViewportFromToRot(const Position& lookFrom, const Position& /* lookAt */, double rotation) {
    myChanger->setViewportFrom(lookFrom.x(), lookFrom.y(), lookFrom.z());
    myChanger->setRotation(rotation);
    update();
}


void
GUISUMOAbstractView::copyViewportTo(GUISUMOAbstractView* view) {
    // look straight down
    view->setViewportFromToRot(Position(myChanger->getXPos(), myChanger->getYPos(), myChanger->getZPos()),
                               Position(myChanger->getXPos(), myChanger->getYPos(), 0),
                               myChanger->getRotation());
}


void
GUISUMOAbstractView::showToolTips(bool val) {
    myUseToolTips = val;
}


bool
GUISUMOAbstractView::setColorScheme(const std::string&) {
    return true;
}


GUIVisualizationSettings&
GUISUMOAbstractView::getVisualisationSettings() const {
    return *myVisualizationSettings;
}


void
GUISUMOAbstractView::remove(GUIDialog_EditViewport*) {
    myViewportChooser = nullptr;
}


void
GUISUMOAbstractView::remove(GUIDialog_ViewSettings*) {
    myVisualizationChanger = nullptr;
}


double
GUISUMOAbstractView::getGridWidth() const {
    return myGrid->getWidth();
}


double
GUISUMOAbstractView::getGridHeight() const {
    return myGrid->getHeight();
}


void
GUISUMOAbstractView::startTrack(int /*id*/) {
}


void
GUISUMOAbstractView::stopTrack() {
}


GUIGlID
GUISUMOAbstractView::getTrackedID() const {
    return GUIGlObject::INVALID_ID;
}


void
GUISUMOAbstractView::onGamingClick(Position /*pos*/) {
}

void
GUISUMOAbstractView::onGamingRightClick(Position /*pos*/) {
}


FXComboBox*
GUISUMOAbstractView::getColoringSchemesCombo() {
    return myParent->getColoringSchemesCombo();
}


FXImage*
GUISUMOAbstractView::checkGDALImage(Decal& d) {
#ifdef HAVE_GDAL
    GDALAllRegister();
    GDALDataset* poDataset = (GDALDataset*)GDALOpen(d.filename.c_str(), GA_ReadOnly);
    if (poDataset == 0) {
        return 0;
    }
    const int xSize = poDataset->GetRasterXSize();
    const int ySize = poDataset->GetRasterYSize();
    // checking for geodata in the picture and try to adapt position and scale
    if (d.width <= 0.) {
        double adfGeoTransform[6];
        if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            Position topLeft(adfGeoTransform[0], adfGeoTransform[3]);
            const double horizontalSize = xSize * adfGeoTransform[1];
            const double verticalSize = ySize * adfGeoTransform[5];
            Position bottomRight(topLeft.x() + horizontalSize, topLeft.y() + verticalSize);
            if (GeoConvHelper::getProcessing().x2cartesian(topLeft) && GeoConvHelper::getProcessing().x2cartesian(bottomRight)) {
                d.width = bottomRight.x() - topLeft.x();
                d.height = topLeft.y() - bottomRight.y();
                d.centerX = (topLeft.x() + bottomRight.x()) / 2;
                d.centerY = (topLeft.y() + bottomRight.y()) / 2;
                //WRITE_MESSAGE("proj: " + toString(poDataset->GetProjectionRef()) + " dim: " + toString(d.width) + "," + toString(d.height) + " center: " + toString(d.centerX) + "," + toString(d.centerY));
            } else {
                WRITE_WARNING("Could not convert coordinates in " + d.filename + ".");
            }
        }
    }
#endif
    if (d.width <= 0.) {
        d.width = getGridWidth();
        d.height = getGridHeight();
    }

    // trying to read the picture
#ifdef HAVE_GDAL
    const int picSize = xSize * ySize;
    FXColor* result;
    if (!FXMALLOC(&result, FXColor, picSize)) {
        WRITE_WARNING("Could not allocate memory for " + d.filename + ".");
        return 0;
    }
    for (int j = 0; j < picSize; j++) {
        result[j] = FXRGB(0, 0, 0);
    }
    bool valid = true;
    for (int i = 1; i <= poDataset->GetRasterCount(); i++) {
        GDALRasterBand* poBand = poDataset->GetRasterBand(i);
        int shift = -1;
        if (poBand->GetColorInterpretation() == GCI_RedBand) {
            shift = 0;
        } else if (poBand->GetColorInterpretation() == GCI_GreenBand) {
            shift = 1;
        } else if (poBand->GetColorInterpretation() == GCI_BlueBand) {
            shift = 2;
        } else if (poBand->GetColorInterpretation() == GCI_AlphaBand) {
            shift = 3;
        } else {
            valid = false;
            break;
        }
        assert(xSize == poBand->GetXSize() && ySize == poBand->GetYSize());
        if (poBand->RasterIO(GF_Read, 0, 0, xSize, ySize, ((unsigned char*)result) + shift, xSize, ySize, GDT_Byte, 4, 4 * xSize) == CE_Failure) {
            valid = false;
            break;
        }
    }
    GDALClose(poDataset);
    if (valid) {
        return new FXImage(getApp(), result, IMAGE_OWNED | IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP, xSize, ySize);
    }
    FXFREE(&result);
#endif
    return nullptr;
}


void
GUISUMOAbstractView::drawDecals() {
    glPushName(0);
    myDecalsLock.lock();
    for (std::vector<GUISUMOAbstractView::Decal>::iterator l = myDecals.begin(); l != myDecals.end(); ++l) {
        GUISUMOAbstractView::Decal& d = *l;
        if (d.skip2D) {
            continue;
        }
        if (!d.initialised) {
            try {
                FXImage* img = checkGDALImage(d);
                if (img == nullptr) {
                    img = MFXImageHelper::loadImage(getApp(), d.filename);
                }
                MFXImageHelper::scalePower2(img, GUITexturesHelper::getMaxTextureSize());
                d.glID = GUITexturesHelper::add(img);
                d.initialised = true;
                d.image = img;
            } catch (InvalidArgument& e) {
                WRITE_ERROR("Could not load '" + d.filename + "'.\n" + e.what());
                d.skip2D = true;
            }
        }
        glPushMatrix();
        if (d.screenRelative) {
            Position center = screenPos2NetPos((int)d.centerX, (int)d.centerY);
            glTranslated(center.x(), center.y(), d.layer);
        } else {
            glTranslated(d.centerX, d.centerY, d.layer);
        }
        glRotated(d.rot, 0, 0, 1);
        glColor3d(1, 1, 1);
        double halfWidth = d.width / 2.;
        double halfHeight = d.height / 2.;
        if (d.screenRelative) {
            halfWidth = p2m(halfWidth);
            halfHeight = p2m(halfHeight);
        }
        GUITexturesHelper::drawTexturedBox(d.glID, -halfWidth, -halfHeight, halfWidth, halfHeight);
        glPopMatrix();
    }
    myDecalsLock.unlock();
    glPopName();
}


// ------------ Additional visualisations
bool
GUISUMOAbstractView::addAdditionalGLVisualisation(GUIGlObject* const which) {
    if (myAdditionallyDrawn.find(which) == myAdditionallyDrawn.end()) {
        myAdditionallyDrawn[which] = 1;
    } else {
        myAdditionallyDrawn[which] = myAdditionallyDrawn[which] + 1;
    }
    update();
    return true;
}


bool
GUISUMOAbstractView::removeAdditionalGLVisualisation(GUIGlObject* const which) {
    if (myAdditionallyDrawn.find(which) == myAdditionallyDrawn.end()) {
        return false;
    }
    int cnt = myAdditionallyDrawn[which];
    if (cnt == 1) {
        myAdditionallyDrawn.erase(which);
    } else {
        myAdditionallyDrawn[which] = myAdditionallyDrawn[which] - 1;
    }
    update();
    return true;
}


bool
GUISUMOAbstractView::isAdditionalGLVisualisationEnabled(GUIGlObject* const which) const {
    if (myAdditionallyDrawn.find(which) == myAdditionallyDrawn.end()) {
        return false;
    } else {
        return true;
    }
}


Boundary
GUISUMOAbstractView::applyGLTransform(bool fixRatio) {
    Boundary bound = myChanger->getViewport(fixRatio);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // as a rough rule, each GLObject is drawn at z = -GUIGlObjectType
    // thus, objects with a higher value will be closer (drawn on top)
    // // @todo last param should be 0 after modifying all glDraw methods
    glOrtho(0, getWidth(), 0, getHeight(), -GLO_MAX - 1, GLO_MAX + 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    double scaleX = (double)getWidth() / bound.getWidth();
    double scaleY = (double)getHeight() / bound.getHeight();
    glScaled(scaleX, scaleY, 1);
    glTranslated(-bound.xmin(), -bound.ymin(), 0);
    // rotate around the center of the screen
    //double angle = -90;
    if (myChanger->getRotation() != 0) {
        glTranslated(bound.getCenter().x(), bound.getCenter().y(), 0);
        glRotated(myChanger->getRotation(), 0, 0, 1);
        glTranslated(-bound.getCenter().x(), -bound.getCenter().y(), 0);
        Boundary rotBound;
        double rad = -DEG2RAD(myChanger->getRotation());
        rotBound.add(Position(bound.xmin(), bound.ymin()).rotateAround2D(rad, bound.getCenter()));
        rotBound.add(Position(bound.xmin(), bound.ymax()).rotateAround2D(rad, bound.getCenter()));
        rotBound.add(Position(bound.xmax(), bound.ymin()).rotateAround2D(rad, bound.getCenter()));
        rotBound.add(Position(bound.xmax(), bound.ymax()).rotateAround2D(rad, bound.getCenter()));
        bound = rotBound;
    }
    myVisualizationSettings->angle = myChanger->getRotation();
    return bound;
}


double
GUISUMOAbstractView::getDelay() const {
    return myApp->getDelay();
}


void
GUISUMOAbstractView::setDelay(double delay) {
    myApp->setDelay(delay);
}


void
GUISUMOAbstractView::setBreakpoints(const std::vector<SUMOTime>& breakpoints) {
    myApp->setBreakpoints(breakpoints);
}


GUISUMOAbstractView::Decal::Decal() :
    filename(),
    centerX(0),
    centerY(0),
    centerZ(0),
    width(0),
    height(0),
    altitude(0),
    rot(0),
    tilt(0),
    roll(0),
    layer(0),
    initialised(false),
    skip2D(false),
    screenRelative(false),
    glID(-1),
    image(nullptr) {
}


/****************************************************************************/
