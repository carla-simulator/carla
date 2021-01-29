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
/// @file    GUIJunctionWrapper.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Andreas Gaubatz
/// @date    Mon, 1 Jul 2003
///
// }
/****************************************************************************/
#include <config.h>

#include <string>
#include <utility>
#ifdef HAVE_OSG
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275) // do not warn about the DLL interface for OSG
#endif
#include <osg/Geometry>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSJunction.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSNet.h>
#include <microsim/MSInternalJunction.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <gui/GUIApplicationWindow.h>
#include <gui/GUIGlobals.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include "GUIJunctionWrapper.h"
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

//#define GUIJunctionWrapper_DEBUG_DRAW_NODE_SHAPE_VERTICES

// ===========================================================================
// method definitions
// ===========================================================================
GUIJunctionWrapper::GUIJunctionWrapper(MSJunction& junction, const std::string& tllID):
    GUIGlObject(GLO_JUNCTION, junction.getID()),
    myJunction(junction),
    myTLLID(tllID) {
    if (myJunction.getShape().size() == 0) {
        Position pos = myJunction.getPosition();
        myBoundary = Boundary(pos.x() - 1., pos.y() - 1., pos.x() + 1., pos.y() + 1.);
    } else {
        myBoundary = myJunction.getShape().getBoxBoundary();
    }
    myMaxSize = MAX2(myBoundary.getWidth(), myBoundary.getHeight());
    myIsInternal = myJunction.getType() == SumoXMLNodeType::INTERNAL;
    myAmWaterway = myJunction.getIncoming().size() + myJunction.getOutgoing().size() > 0;
    myAmRailway = myJunction.getIncoming().size() + myJunction.getOutgoing().size() > 0;
    for (auto it = myJunction.getIncoming().begin(); it != myJunction.getIncoming().end() && (myAmWaterway || myAmRailway); ++it) {
        if (!(*it)->isInternal()) {
            if (!isWaterway((*it)->getPermissions())) {
                myAmWaterway = false;
            }
            if (!isRailway((*it)->getPermissions())) {
                myAmRailway = false;
            }
        }
    }
    for (auto it = myJunction.getOutgoing().begin(); it != myJunction.getOutgoing().end() && (myAmWaterway || myAmRailway); ++it) {
        if (!(*it)->isInternal()) {
            if (!isWaterway((*it)->getPermissions())) {
                myAmWaterway = false;
            }
            if (!isRailway((*it)->getPermissions())) {
                myAmRailway = false;
            }
        }
    }
}


GUIJunctionWrapper::~GUIJunctionWrapper() {}


GUIGLObjectPopupMenu*
GUIJunctionWrapper::getPopUpMenu(GUIMainWindow& app,
                                 GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUIJunctionWrapper::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("type", false, toString(myJunction.getType()));
    ret->mkItem("name", false, myJunction.getName());
    // close building
    ret->closeBuilding(&myJunction);
    return ret;
}


Boundary
GUIJunctionWrapper::getCenteringBoundary() const {
    Boundary b = myBoundary;
    b.grow(1);
    return b;
}

const std::string
GUIJunctionWrapper::getOptionalName() const {
    return myJunction.getParameter("name", "");
}

void
GUIJunctionWrapper::drawGL(const GUIVisualizationSettings& s) const {
    if (!myIsInternal && s.drawJunctionShape) {
        // check whether it is not too small
        const double exaggeration = s.junctionSize.getExaggeration(s, this, 4);
        if (s.scale * exaggeration >= s.junctionSize.minSize) {
            glPushMatrix();
            glPushName(getGlID());
            const double colorValue = getColorValue(s, s.junctionColorer.getActive());
            const RGBColor color = s.junctionColorer.getScheme().getColor(colorValue);
            GLHelper::setColor(color);

            // recognize full transparency and simply don't draw
            if (color.alpha() != 0) {
                PositionVector shape = myJunction.getShape();
                shape.closePolygon();
                if (exaggeration > 1) {
                    shape.scaleRelative(exaggeration);
                }
                glTranslated(0, 0, getType());
                if (s.scale * myMaxSize < 40.) {
                    GLHelper::drawFilledPoly(shape, true);
                } else {
                    GLHelper::drawFilledPolyTesselated(shape, true);
                }
#ifdef GUIJunctionWrapper_DEBUG_DRAW_NODE_SHAPE_VERTICES
                GLHelper::debugVertices(shape, 80 / s.scale);
#endif
                // make small junctions more visible when coloring by type
                if (myJunction.getType() == SumoXMLNodeType::RAIL_SIGNAL && s.junctionColorer.getActive() == 2) {
                    glTranslated(myJunction.getPosition().x(), myJunction.getPosition().y(), getType() + 0.05);
                    GLHelper::drawFilledCircle(2 * exaggeration, 12);
                }
            }
            glPopName();
            glPopMatrix();
        }
    }
    if (myIsInternal) {
        drawName(myJunction.getPosition(), s.scale, s.internalJunctionName, s.angle);
    } else {
        drawName(myJunction.getPosition(), s.scale, s.junctionID, s.angle);
        if (s.junctionName.show && myJunction.getName() != "") {
            GLHelper::drawTextSettings(s.junctionName, myJunction.getName(), myJunction.getPosition(), s.scale, s.angle);
        }
        if ((s.tlsPhaseIndex.show || s.tlsPhaseName.show) && myTLLID != "") {
            const MSTrafficLightLogic* active = MSNet::getInstance()->getTLSControl().getActive(myTLLID);
            if (s.tlsPhaseIndex.show) {
                const int index = active->getCurrentPhaseIndex();
                GLHelper::drawTextSettings(s.tlsPhaseIndex, toString(index), myJunction.getPosition(), s.scale, s.angle);
            }
            if (s.tlsPhaseName.show) {
                const std::string& name = active->getCurrentPhaseDef().getName();
                if (name != "") {
                    const Position offset = (s.tlsPhaseIndex.show ?
                                             Position(0, 0.8 * s.tlsPhaseIndex.scaledSize(s.scale)).rotateAround2D(DEG2RAD(-s.angle), Position(0, 0))
                                             : Position(0, 0));
                    GLHelper::drawTextSettings(s.tlsPhaseName, name, myJunction.getPosition() - offset, s.scale, s.angle);
                }
            }
        }
    }
}


double
GUIJunctionWrapper::getColorValue(const GUIVisualizationSettings& /* s */, int activeScheme) const {
    switch (activeScheme) {
        case 0:
            if (myAmWaterway) {
                return 1;
            } else if (myAmRailway && MSNet::getInstance()->hasInternalLinks()) {
                return 2;
            } else {
                return 0;
            }
        case 1:
            return gSelected.isSelected(getType(), getGlID()) ? 1 : 0;
        case 2:
            switch (myJunction.getType()) {
                case SumoXMLNodeType::TRAFFIC_LIGHT:
                    return 0;
                case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
                    return 1;
                case SumoXMLNodeType::PRIORITY:
                    return 2;
                case SumoXMLNodeType::PRIORITY_STOP:
                    return 3;
                case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
                    return 4;
                case SumoXMLNodeType::ALLWAY_STOP:
                    return 5;
                case SumoXMLNodeType::DISTRICT:
                    return 6;
                case SumoXMLNodeType::NOJUNCTION:
                    return 7;
                case SumoXMLNodeType::DEAD_END:
                case SumoXMLNodeType::DEAD_END_DEPRECATED:
                    return 8;
                case SumoXMLNodeType::UNKNOWN:
                case SumoXMLNodeType::INTERNAL:
                    assert(false);
                    return 8;
                case SumoXMLNodeType::RAIL_SIGNAL:
                    return 9;
                case SumoXMLNodeType::ZIPPER:
                    return 10;
                case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED:
                    return 11;
                case SumoXMLNodeType::RAIL_CROSSING:
                    return 12;
            }
        case 3:
            return myJunction.getPosition().z();
        default:
            assert(false);
            return 0;
    }
}

#ifdef HAVE_OSG
void
GUIJunctionWrapper::updateColor(const GUIVisualizationSettings& s) {
    const double colorValue = getColorValue(s, s.junctionColorer.getActive());
    const RGBColor& col = s.junctionColorer.getScheme().getColor(colorValue);
    osg::Vec4ubArray* colors = dynamic_cast<osg::Vec4ubArray*>(myGeom->getColorArray());
    (*colors)[0].set(col.red(), col.green(), col.blue(), col.alpha());
    myGeom->setColorArray(colors);
}
#endif


/****************************************************************************/
