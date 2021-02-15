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
/// @file    GUIPolygon.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    June 2006
///
// The GUI-version of a polygon
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include "GUIPolygon.h"

//#define GUIPolygon_DEBUG_DRAW_VERTICES

// ===========================================================================
// callbacks definitions
// ===========================================================================

void APIENTRY beginCallback(GLenum which) {
    glBegin(which);
}


void APIENTRY errorCallback(GLenum errorCode) {
    const GLubyte* estring;

    estring = gluErrorString(errorCode);
    fprintf(stderr, "Tessellation Error: %s\n", estring);
    exit(0);
}


void APIENTRY endCallback(void) {
    glEnd();
}


void APIENTRY vertexCallback(GLvoid* vertex) {
    glVertex3dv((GLdouble*) vertex);
}


void APIENTRY combineCallback(GLdouble coords[3],
                              GLdouble* vertex_data[4],
                              GLfloat weight[4], GLdouble** dataOut) {
    UNUSED_PARAMETER(weight);
    UNUSED_PARAMETER(*vertex_data);
    GLdouble* vertex;

    vertex = (GLdouble*) malloc(7 * sizeof(GLdouble));

    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *dataOut = vertex;
}

GLfloat INV_POLY_TEX_DIM = 1.0 / 256.0;
GLfloat xPlane[] = {INV_POLY_TEX_DIM, 0.0, 0.0, 0.0};
GLfloat yPlane[] = {0.0, INV_POLY_TEX_DIM, 0.0, 0.0};

// ===========================================================================
// method definitions
// ===========================================================================

GUIPolygon::GUIPolygon(const std::string& id, const std::string& type,
                       const RGBColor& color, const PositionVector& shape, bool geo,
                       bool fill, double lineWidth, double layer, double angle, const std::string& imgFile,
                       bool relativePath):
    SUMOPolygon(id, type, color, shape, geo, fill, lineWidth, layer, angle, imgFile, relativePath),
    GUIGlObject_AbstractAdd(GLO_POLYGON, id),
    myDisplayList(0),
    myRotatedShape(nullptr) {
    if (angle != 0.) {
        setShape(shape);
    }
}


GUIPolygon::~GUIPolygon() {
    delete myRotatedShape;
}



GUIGLObjectPopupMenu*
GUIPolygon::getPopUpMenu(GUIMainWindow& app,
                         GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app, false);
    FXString t(getShapeType().c_str());
    new FXMenuCommand(ret, "(" + t + ")", nullptr, nullptr, 0);
    new FXMenuSeparator(ret);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    buildShowParamsPopupEntry(ret, false);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUIPolygon::getParameterWindow(GUIMainWindow& app,
                               GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("type", false, getShapeType());
    ret->mkItem("layer", false, toString(getShapeLayer()));
    ret->closeBuilding(this);
    return ret;
}


Boundary
GUIPolygon::getCenteringBoundary() const {
    const PositionVector& shape = myRotatedShape != nullptr ? *myRotatedShape : myShape;
    Boundary b;
    b.add(shape.getBoxBoundary());
    b.grow(2);
    return b;
}


void
GUIPolygon::drawGL(const GUIVisualizationSettings& s) const {
    // first check if polygon can be drawn
    if (checkDraw(s)) {
        FXMutexLock locker(myLock);
        //if (myDisplayList == 0 || (!getFill() && myLineWidth != s.polySize.getExaggeration(s))) {
        //    storeTesselation(s.polySize.getExaggeration(s));
        //}
        // push name (needed for getGUIGlObjectsUnderCursor(...)
        glPushName(getGlID());
        // draw inner polygon
        drawInnerPolygon(s, (myRotatedShape != nullptr ? *myRotatedShape : myShape), getShapeLayer(), false);
        // pop name
        glPopName();
    }
}


void
GUIPolygon::setShape(const PositionVector& shape) {
    FXMutexLock locker(myLock);
    SUMOPolygon::setShape(shape);
    if (getShapeNaviDegree() != 0.) {
        if (myRotatedShape == nullptr) {
            myRotatedShape = new PositionVector();
        }
        const Position& centroid = myShape.getCentroid();
        *myRotatedShape = myShape;
        myRotatedShape->sub(centroid);
        myRotatedShape->rotate2D(-DEG2RAD(getShapeNaviDegree()));
        myRotatedShape->add(centroid);
    } else {
        delete myRotatedShape;
        myRotatedShape = nullptr;
    }
    //storeTesselation(myLineWidth);
}


void
GUIPolygon::performTesselation(const PositionVector& shape, double lineWidth) const {
    if (getFill()) {
        // draw the tesselated shape
        double* points = new double[shape.size() * 3];
        GLUtesselator* tobj = gluNewTess();
        gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid(APIENTRY*)()) &glVertex3dv);
        gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid(APIENTRY*)()) &beginCallback);
        gluTessCallback(tobj, GLU_TESS_END, (GLvoid(APIENTRY*)()) &endCallback);
        //gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (APIENTRY*) ()) &errorCallback);
        gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid(APIENTRY*)()) &combineCallback);
        gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        gluTessBeginPolygon(tobj, nullptr);
        gluTessBeginContour(tobj);
        for (int i = 0; i != (int)shape.size(); ++i) {
            points[3 * i]  = shape[(int) i].x();
            points[3 * i + 1]  = shape[(int) i].y();
            points[3 * i + 2]  = 0;
            gluTessVertex(tobj, points + 3 * i, points + 3 * i);
        }
        gluTessEndContour(tobj);

        gluTessEndPolygon(tobj);
        gluDeleteTess(tobj);
        delete[] points;

    } else {
        GLHelper::drawLine(shape);
        GLHelper::drawBoxLines(shape, lineWidth);
    }
    //std::cout << "OpenGL says: '" << gluErrorString(glGetError()) << "'\n";
}


void
GUIPolygon::storeTesselation(const PositionVector& shape, double lineWidth) const {
    if (myDisplayList > 0) {
        glDeleteLists(myDisplayList, 1);
    }
    myDisplayList = glGenLists(1);
    if (myDisplayList == 0) {
        throw ProcessError("GUIPolygon::storeTesselation() could not create display list");
    }
    glNewList(myDisplayList, GL_COMPILE);
    performTesselation(shape, lineWidth);
    glEndList();
}


void
GUIPolygon::setColor(const GUIVisualizationSettings& s, bool disableSelectionColor) const {
    const GUIColorer& c = s.polyColorer;
    const int active = c.getActive();
    if (s.netedit && active != 1 && gSelected.isSelected(GLO_POLYGON, getGlID()) && disableSelectionColor) {
        // override with special selection colors (unless the color scheme is based on selection)
        GLHelper::setColor(RGBColor(0, 0, 204));
    } else if (active == 0) {
        GLHelper::setColor(getShapeColor());
    } else if (active == 1) {
        GLHelper::setColor(c.getScheme().getColor(gSelected.isSelected(GLO_POLYGON, getGlID())));
    } else {
        GLHelper::setColor(c.getScheme().getColor(0));
    }
}


bool
GUIPolygon::checkDraw(const GUIVisualizationSettings& s) const {
    if (s.polySize.getExaggeration(s, this) == 0) {
        return false;
    }
    Boundary boundary = myShape.getBoxBoundary();
    if (s.scale * MAX2(boundary.getWidth(), boundary.getHeight()) < s.polySize.minSize) {
        return false;
    }
    if (getFill()) {
        if (myShape.size() < 3) {
            return false;
        }
    } else {
        if (myShape.size() < 2) {
            return false;
        }
    }
    return true;
}


void
GUIPolygon::drawInnerPolygon(const GUIVisualizationSettings& s, const PositionVector& shape, double layer, bool disableSelectionColor) const {
    glPushMatrix();
    glTranslated(0, 0, layer);
    setColor(s, disableSelectionColor);

    int textureID = -1;
    if (getFill()) {
        const std::string& file = getShapeImgFile();
        if (file != "") {
            textureID = GUITexturesHelper::getTextureID(file, true);
        }
    }
    // init generation of texture coordinates
    if (textureID >= 0) {
        glEnable(GL_TEXTURE_2D);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST); // without DEPTH_TEST vehicles may be drawn below roads
        glDisable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // http://www.gamedev.net/topic/133564-glutesselation-and-texture-mapping/
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_S, GL_OBJECT_PLANE, xPlane);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, yPlane);
    }
    // recall tesselation
    //glCallList(myDisplayList);
    performTesselation(shape, myLineWidth * s.polySize.getExaggeration(s, this));
    // de-init generation of texture coordinates
    if (textureID >= 0) {
        glEnable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
#ifdef GUIPolygon_DEBUG_DRAW_VERTICES
    GLHelper::debugVertices(shape, 80 / s.scale);
#endif
    glPopMatrix();
    const Position& namePos = shape.getPolygonCenter();
    drawName(namePos, s.scale, s.polyName, s.angle);
    if (s.polyType.show) {
        const Position p = namePos + Position(0, -0.6 * s.polyType.size / s.scale);
        GLHelper::drawTextSettings(s.polyType, getShapeType(), p, s.scale, s.angle);
    }
}


/****************************************************************************/
