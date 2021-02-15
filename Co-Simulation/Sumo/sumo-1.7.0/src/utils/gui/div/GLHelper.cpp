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
/// @file    GLHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Some methods which help to draw certain geometrical objects in openGL
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utils/geom/GeomHelper.h>
#include <utils/common/StdDefs.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#define FONTSTASH_IMPLEMENTATION // Expands implementation
#ifdef _MSC_VER
#pragma warning(disable: 4505) // do not warn about unused functions
#endif
#if __GNUC__ > 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <foreign/fontstash/fontstash.h>
#include <utils/gui/globjects/GLIncludes.h>
#define GLFONTSTASH_IMPLEMENTATION // Expands implementation
#include <foreign/fontstash/glfontstash.h>
#include <utils/geom/Boundary.h>
#ifdef HAVE_GL2PS
#include <gl2ps.h>
#endif
#include "Roboto.h"
#include "GLHelper.h"

#define CIRCLE_RESOLUTION (double)10 // inverse in degrees

// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::pair<double, double> > GLHelper::myCircleCoords;
std::vector<RGBColor> GLHelper::myDottedcontourColors;
FONScontext* GLHelper::myFont = nullptr;
double GLHelper::myFontSize = 50.0;
bool GLHelper::myGL2PSActive = false;

void APIENTRY combCallback(GLdouble coords[3],
                           GLdouble* vertex_data[4],
                           GLfloat weight[4], GLdouble** dataOut) {
    UNUSED_PARAMETER(weight);
    UNUSED_PARAMETER(*vertex_data);
    GLdouble* vertex;

    vertex = (GLdouble*)malloc(7 * sizeof(GLdouble));

    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *dataOut = vertex;
}

// ===========================================================================
// method definitions
// ===========================================================================


void
GLHelper::drawFilledPoly(const PositionVector& v, bool close) {
    if (v.size() == 0) {
        return;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON);
    for (PositionVector::const_iterator i = v.begin(); i != v.end(); i++) {
        const Position& p = *i;
        glVertex2d(p.x(), p.y());
    }
    if (close) {
        const Position& p = *(v.begin());
        glVertex2d(p.x(), p.y());
    }
    glEnd();
}


void
GLHelper::drawFilledPolyTesselated(const PositionVector& v, bool close) {
    if (v.size() == 0) {
        return;
    }
    GLUtesselator* tobj = gluNewTess();
    gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid(APIENTRY*)()) &glVertex3dv);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid(APIENTRY*)()) &glBegin);
    gluTessCallback(tobj, GLU_TESS_END, (GLvoid(APIENTRY*)()) &glEnd);
    gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid(APIENTRY*)()) &combCallback);
    gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    gluTessBeginPolygon(tobj, nullptr);
    gluTessBeginContour(tobj);
    double* points = new double[(v.size() + int(close)) * 3];

    for (int i = 0; i != (int)v.size(); ++i) {
        points[3 * i] = v[i].x();
        points[3 * i + 1] = v[i].y();
        points[3 * i + 2] = 0;
        gluTessVertex(tobj, points + 3 * i, points + 3 * i);
    }
    if (close) {
        const int i = (int)v.size();
        points[3 * i] = v[0].x();
        points[3 * i + 1] = v[0].y();
        points[3 * i + 2] = 0;
        gluTessVertex(tobj, points + 3 * i, points + 3 * i);
    }
    gluTessEndContour(tobj);
    gluTessEndPolygon(tobj);
    gluDeleteTess(tobj);
    delete[] points;
}


void
GLHelper::drawBoxLine(const Position& beg, double rot, double visLength,
                      double width, double offset) {
    glPushMatrix();
    glTranslated(beg.x(), beg.y(), 0);
    glRotated(rot, 0, 0, 1);
    glBegin(GL_QUADS);
    glVertex2d(-width - offset, 0);
    glVertex2d(-width - offset, -visLength);
    glVertex2d(width - offset, -visLength);
    glVertex2d(width - offset, 0);
    glEnd();
    glPopMatrix();
}


void
GLHelper::drawBoxLine(const Position& beg1, const Position& beg2,
                      double rot, double visLength,
                      double width) {
    glPushMatrix();
    glTranslated((beg2.x() + beg1.x())*.5, (beg2.y() + beg1.y())*.5, 0);
    glRotated(rot, 0, 0, 1);
    glBegin(GL_QUADS);
    glVertex2d(-width, 0);
    glVertex2d(-width, -visLength);
    glVertex2d(width, -visLength);
    glVertex2d(width, 0);
    glEnd();
    glPopMatrix();
}


bool
GLHelper::rightTurn(double angle1, double angle2) {
    double delta = angle2 - angle1;
    while (delta > 180) {
        delta -= 360;
    }
    while (delta < -180) {
        delta += 360;
    }
    return delta <= 0;
}


void
GLHelper::drawBoxLines(const PositionVector& geom,
                       const std::vector<double>& rots,
                       const std::vector<double>& lengths,
                       double width, int cornerDetail, double offset) {
    // draw the lane
    int e = (int) geom.size() - 1;
    for (int i = 0; i < e; i++) {
        drawBoxLine(geom[i], rots[i], lengths[i], width, offset);
    }
    // draw the corner details
    if (cornerDetail > 0) {
        for (int i = 1; i < e; i++) {
            glPushMatrix();
            glTranslated(geom[i].x(), geom[i].y(), 0.1);
            double angleBeg = -rots[i - 1];
            double angleEnd = 180 - rots[i];
            if (rightTurn(rots[i - 1], rots[i])) {
                std::swap(angleBeg, angleEnd);
            }
            // only draw the missing piece
            angleBeg -= 90;
            angleEnd += 90;
            // avoid drawing more than 360 degrees
            if (angleEnd - angleBeg > 360) {
                angleBeg += 360;
            }
            if (angleEnd - angleBeg < -360) {
                angleEnd += 360;
            }
            // draw the right way around
            if (angleEnd > angleBeg) {
                angleEnd -= 360;
            }
            drawFilledCircle(width + offset, cornerDetail, angleBeg, angleEnd);
            glPopMatrix();
        }
    }
}


void
GLHelper::drawBoxLines(const PositionVector& geom,
                       const std::vector<double>& rots,
                       const std::vector<double>& lengths,
                       const std::vector<RGBColor>& cols,
                       double width, int cornerDetail, double offset) {
    int e = (int) geom.size() - 1;
    for (int i = 0; i < e; i++) {
        setColor(cols[i]);
        drawBoxLine(geom[i], rots[i], lengths[i], width, offset);
    }
    if (cornerDetail > 0) {
        for (int i = 1; i < e; i++) {
            glPushMatrix();
            setColor(cols[i]);
            glTranslated(geom[i].x(), geom[i].y(), 0);
            drawFilledCircle(width, cornerDetail);
            glEnd();
            glPopMatrix();
        }
    }
}


void
GLHelper::drawBoxLines(const PositionVector& geom1,
                       const PositionVector& geom2,
                       const std::vector<double>& rots,
                       const std::vector<double>& lengths,
                       double width) {
    int minS = (int) MIN4(rots.size(), lengths.size(), geom1.size(), geom2.size());
    for (int i = 0; i < minS; i++) {
        GLHelper::drawBoxLine(geom1[i], geom2[i], rots[i], lengths[i], width);
    }
}


void
GLHelper::drawBoxLines(const PositionVector& geom, double width) {
    int e = (int) geom.size() - 1;
    for (int i = 0; i < e; i++) {
        const Position& f = geom[i];
        const Position& s = geom[i + 1];
        drawBoxLine(f,
                    RAD2DEG(atan2((s.x() - f.x()), (f.y() - s.y()))),
                    f.distanceTo(s),
                    width);
    }
}


void
GLHelper::drawLine(const Position& beg, double rot, double visLength) {
    glPushMatrix();
    glTranslated(beg.x(), beg.y(), 0);
    glRotated(rot, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(0, -visLength);
    glEnd();
    glPopMatrix();
}


void
GLHelper::drawLine(const Position& beg1, const Position& beg2,
                   double rot, double visLength) {
    glPushMatrix();
    glTranslated((beg2.x() + beg1.x())*.5, (beg2.y() + beg1.y())*.5, 0);
    glRotated(rot, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(0, -visLength);
    glEnd();
    glPopMatrix();
}



void
GLHelper::drawLine(const PositionVector& v) {
    glBegin(GL_LINES);
    int e = (int) v.size() - 1;
    for (int i = 0; i < e; ++i) {
        glVertex2d(v[i].x(), v[i].y());
        glVertex2d(v[i + 1].x(), v[i + 1].y());
    }
    glEnd();
}


void
GLHelper::drawLine(const PositionVector& v, const std::vector<RGBColor>& cols) {
    glBegin(GL_LINES);
    int e = (int) v.size() - 1;
    for (int i = 0; i < e; ++i) {
        setColor(cols[i]);
        glVertex2d(v[i].x(), v[i].y());
        glVertex2d(v[i + 1].x(), v[i + 1].y());
    }
    glEnd();
}


void
GLHelper::drawLine(const Position& beg, const Position& end) {
    glBegin(GL_LINES);
    glVertex2d(beg.x(), beg.y());
    glVertex2d(end.x(), end.y());
    glEnd();
}


int
GLHelper::angleLookup(double angleDeg) {
    const int numCoords = (int)myCircleCoords.size() - 1;
    int index = ((int)(floor(angleDeg * CIRCLE_RESOLUTION + 0.5))) % numCoords;
    if (index < 0) {
        index += numCoords;
    }
    assert(index >= 0);
    return (int)index;
}


void
GLHelper::drawFilledCircle(double width, int steps) {
    drawFilledCircle(width, steps, 0, 360);
}


void
GLHelper::drawFilledCircle(double width, int steps, double beg, double end) {
    if (myCircleCoords.size() == 0) {
        for (int i = 0; i <= (int)(360 * CIRCLE_RESOLUTION); ++i) {
            const double x = (double) sin(DEG2RAD(i / CIRCLE_RESOLUTION));
            const double y = (double) cos(DEG2RAD(i / CIRCLE_RESOLUTION));
            myCircleCoords.push_back(std::pair<double, double>(x, y));
        }
    }
    const double inc = (end - beg) / (double)steps;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    std::pair<double, double> p1 = myCircleCoords[angleLookup(beg)];

    for (int i = 0; i <= steps; ++i) {
        const std::pair<double, double>& p2 = myCircleCoords[angleLookup(beg + i * inc)];
        glBegin(GL_TRIANGLES);
        glVertex2d(p1.first * width, p1.second * width);
        glVertex2d(p2.first * width, p2.second * width);
        glVertex2d(0, 0);
        glEnd();
        p1 = p2;
    }
}


void
GLHelper::drawOutlineCircle(double width, double iwidth, int steps) {
    drawOutlineCircle(width, iwidth, steps, 0, 360);
}


void
GLHelper::drawOutlineCircle(double width, double iwidth, int steps,
                            double beg, double end) {
    if (myCircleCoords.size() == 0) {
        for (int i = 0; i < 360; i += 10) {
            double x = (double) sin(DEG2RAD(i));
            double y = (double) cos(DEG2RAD(i));
            myCircleCoords.push_back(std::pair<double, double>(x, y));
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    std::pair<double, double> p1 =
        beg == 0 ? myCircleCoords[0] : myCircleCoords[((int) beg / 10) % 36];
    for (int i = (int)(beg / 10); i < steps && (36.0 / (double) steps * (double) i) * 10 < end; i++) {
        const std::pair<double, double>& p2 =
            myCircleCoords[(int)(36.0 / (double) steps * (double) i)];
        glBegin(GL_TRIANGLES);
        glVertex2d(p1.first * width, p1.second * width);
        glVertex2d(p2.first * width, p2.second * width);
        glVertex2d(p2.first * iwidth, p2.second * iwidth);

        glVertex2d(p2.first * iwidth, p2.second * iwidth);
        glVertex2d(p1.first * iwidth, p1.second * iwidth);
        glVertex2d(p1.first * width, p1.second * width);
        glEnd();
        p1 = p2;
    }
    const std::pair<double, double>& p2 =
        end == 360 ? myCircleCoords[0] : myCircleCoords[((int) end / 10) % 36];
    glBegin(GL_TRIANGLES);
    glVertex2d(p1.first * width, p1.second * width);
    glVertex2d(p2.first * width, p2.second * width);
    glVertex2d(p2.first * iwidth, p2.second * iwidth);

    glVertex2d(p2.first * iwidth, p2.second * iwidth);
    glVertex2d(p1.first * iwidth, p1.second * iwidth);
    glVertex2d(p1.first * width, p1.second * width);
    glEnd();
}


void
GLHelper::drawTriangleAtEnd(const Position& p1, const Position& p2,
                            double tLength, double tWidth) {
    const double length = p1.distanceTo(p2);
    if (length < tLength) {
        tWidth *= length / tLength;
        tLength = length;
    }
    Position rl(PositionVector::positionAtOffset(p1, p2, length - tLength));
    glPushMatrix();
    glTranslated(rl.x(), rl.y(), 0);
    glRotated(-GeomHelper::naviDegree(p1.angleTo2D(p2)), 0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex2d(0, tLength);
    glVertex2d(-tWidth, 0);
    glVertex2d(+tWidth, 0);
    glEnd();
    glPopMatrix();
}


void
GLHelper::setColor(const RGBColor& c) {
    glColor4ub(c.red(), c.green(), c.blue(), c.alpha());
}


RGBColor
GLHelper::getColor() {
    GLdouble current[4];
    glGetDoublev(GL_CURRENT_COLOR, current);
    return RGBColor(static_cast<unsigned char>(current[0] * 255. + 0.5),
                    static_cast<unsigned char>(current[1] * 255. + 0.5),
                    static_cast<unsigned char>(current[2] * 255. + 0.5),
                    static_cast<unsigned char>(current[3] * 255. + 0.5));
}


void
GLHelper::resetFont() {
    glfonsDelete(myFont);
    myFont = nullptr;
}


bool
GLHelper::initFont() {
    if (myFont == nullptr) {
        myFont = glfonsCreate(2048, 2048, FONS_ZERO_BOTTOMLEFT);
        if (myFont != nullptr) {
            const int fontNormal = fonsAddFontMem(myFont, "medium", data_font_Roboto_Medium_ttf, data_font_Roboto_Medium_ttf_len, 0);
            fonsSetFont(myFont, fontNormal);
            fonsSetSize(myFont, (float)myFontSize);
        }
    }
    return myFont != nullptr;
}


const std::vector<RGBColor>&
GLHelper::getDottedcontourColors(const int size) {
    // check if more colors has to be added
    while ((int)myDottedcontourColors.size() < size) {
        if (myDottedcontourColors.empty() || myDottedcontourColors.back() == RGBColor::WHITE) {
            myDottedcontourColors.push_back(RGBColor::BLACK);
        } else {
            myDottedcontourColors.push_back(RGBColor::WHITE);
        }
    }
    return myDottedcontourColors;
}


void
GLHelper::drawText(const std::string& text, const Position& pos, const double layer, const double size,
                   const RGBColor& col, const double angle, const int align, double width) {
    if (width <= 0) {
        width = size;
    }
    if (!initFont()) {
        return;
    }
    glPushMatrix();
    glAlphaFunc(GL_GREATER, 0.5);
    glEnable(GL_ALPHA_TEST);
#ifdef HAVE_GL2PS
    if (myGL2PSActive) {
        glRasterPos3d(pos.x(), pos.y(), layer);
        GLfloat color[] = {col.red() / 255.f, col.green() / 255.f, col.blue() / 255.f, col.alpha() / 255.f};
        gl2psTextOptColor(text.c_str(), "Roboto", 10, align == 0 ? GL2PS_TEXT_C : align, (GLfloat) - angle, color);
        glPopMatrix();
        return;
    }
#endif
    glTranslated(pos.x(), pos.y(), layer);
    glScaled(width / myFontSize, size / myFontSize, 1.);
    glRotated(-angle, 0, 0, 1);
    fonsSetAlign(myFont, align == 0 ? FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE : align);
    fonsSetColor(myFont, glfonsRGBA(col.red(), col.green(), col.blue(), col.alpha()));
    fonsDrawText(myFont, 0., 0., text.c_str(), nullptr);
    glPopMatrix();
}


void
GLHelper::drawTextSettings(
    const GUIVisualizationTextSettings& settings,
    const std::string& text, const Position& pos,
    const double scale,
    const double angle,
    const double layer,
    const int align) {
    drawTextBox(text, pos, layer,
                settings.scaledSize(scale),
                settings.color,
                settings.bgColor,
                RGBColor::INVISIBLE,
                angle, 0, 0.2, align);
}


void
GLHelper::drawTextBox(const std::string& text, const Position& pos,
                      const double layer, const double size,
                      const RGBColor& txtColor, const RGBColor& bgColor, const RGBColor& borderColor,
                      const double angle,
                      const double relBorder,
                      const double relMargin,
                      const int align) {
    if (!initFont()) {
        return;
    };
    if (bgColor.alpha() != 0) {
        const double boxAngle = 90;
        const double stringWidth = size / myFontSize * fonsTextBounds(myFont, 0, 0, text.c_str(), nullptr, nullptr);
        const double borderWidth = size * relBorder;
        const double boxHeight = size * (0.32 + 0.6 * relMargin);
        const double boxWidth = stringWidth + size * relMargin;
        glPushMatrix();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glTranslated(pos.x(), pos.y(), layer);
        glRotated(-angle, 0, 0, 1);
        Position left(-boxWidth * 0.5, 0);
        setColor(borderColor);
        drawBoxLine(left, boxAngle, boxWidth, boxHeight);
        left.add(borderWidth * 1.5, 0);
        setColor(bgColor);
        glTranslated(0, 0, 0.01);
        drawBoxLine(left, boxAngle, boxWidth - 3 * borderWidth, boxHeight - 2 * borderWidth);
        glPopMatrix();
    }
    drawText(text, pos, layer + 0.02, size, txtColor, angle, align);
}


void
GLHelper::drawTextAtEnd(const std::string& text, const PositionVector& shape, double x,
                        const GUIVisualizationTextSettings& settings, const double scale) {
    glPushMatrix();
    const Position& end = shape.back();
    const Position& f = shape[-2];
    const double rot = RAD2DEG(atan2((end.x() - f.x()), (f.y() - end.y())));
    glTranslated(end.x(), end.y(), 0);
    glRotated(rot, 0, 0, 1);
    drawTextBox(text, Position(x, 0.26), 0,
                settings.scaledSize(scale, 0.01),
                settings.color,
                settings.bgColor,
                RGBColor::INVISIBLE,
                180, 0, 0.2);
    glPopMatrix();
}

void
GLHelper::drawCrossTies(const PositionVector& geom,
                        const std::vector<double>& rots,
                        const std::vector<double>& lengths,
                        double length, double spacing,
                        double halfWidth, bool drawForRectangleSelection) {
    glPushMatrix();
    // draw on top of of the white area between the rails
    glTranslated(0, 0, 0.1);
    int e = (int) geom.size() - 1;
    for (int i = 0; i < e; ++i) {
        glPushMatrix();
        glTranslated(geom[i].x(), geom[i].y(), 0.0);
        glRotated(rots[i], 0, 0, 1);
        // draw crossing depending if isn't being drawn for selecting
        if (!drawForRectangleSelection) {
            for (double t = 0; t < lengths[i]; t += spacing) {
                glBegin(GL_QUADS);
                glVertex2d(-halfWidth, -t);
                glVertex2d(-halfWidth, -t - length);
                glVertex2d(halfWidth, -t - length);
                glVertex2d(halfWidth, -t);
                glEnd();
            }
        } else {
            // only draw a single rectangle if it's being drawn only for selecting
            glBegin(GL_QUADS);
            glVertex2d(-halfWidth, 0);
            glVertex2d(-halfWidth, -lengths.back());
            glVertex2d(halfWidth, -lengths.back());
            glVertex2d(halfWidth, 0);
            glEnd();
        }
        // pop three draw matrix
        glPopMatrix();
    }
    glPopMatrix();
}


void
GLHelper::debugVertices(const PositionVector& shape, double size, double layer) {
    RGBColor color = RGBColor::randomHue();
    for (int i = 0; i < (int)shape.size(); ++i) {
        GLHelper::drawText(toString(i), shape[i], layer, size, color, 0);
    }
}


void
GLHelper::drawBoundary(const Boundary& b) {
    glPushMatrix();
    GLHelper::setColor(RGBColor::MAGENTA);
    // draw on top
    glTranslated(0, 0, 1024);
    drawLine(Position(b.xmin(), b.ymax()), Position(b.xmax(), b.ymax()));
    drawLine(Position(b.xmax(), b.ymax()), Position(b.xmax(), b.ymin()));
    drawLine(Position(b.xmax(), b.ymin()), Position(b.xmin(), b.ymin()));
    drawLine(Position(b.xmin(), b.ymin()), Position(b.xmin(), b.ymax()));
    glPopMatrix();
}


/****************************************************************************/
