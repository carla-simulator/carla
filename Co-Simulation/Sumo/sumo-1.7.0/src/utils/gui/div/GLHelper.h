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
/// @file    GLHelper.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Some methods which help to draw certain geometrical objects in openGL
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <utility>
#include <utils/common/RGBColor.h>
#include <utils/geom/PositionVector.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>


// ===========================================================================
// class declarations
// ===========================================================================
struct GUIVisualizationTextSettings;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GLHelper
 * @brief Some methods which help to draw certain geometrical objects in openGL
 *
 * This class offers some static methods for drawing primitives in openGL.
 */
class GLHelper {
public:
    /** @brief Draws a filled polygon described by the list of points
     * @note this only works well for convex polygons
     *
     * @param[in] v The polygon to draw
     * @param[in] close Whether the first point shall be appended
     */
    static void drawFilledPoly(const PositionVector& v, bool close);


    /** @brief Draws a filled polygon described by the list of points
     * @note this works for convex and concave polygons but is slower than
     * drawFilledPoly
     *
     * @param[in] v The polygon to draw
     * @param[in] close Whether the first point shall be appended
     */
    static void drawFilledPolyTesselated(const PositionVector& v, bool close);


    /** @brief Draws a thick line
     *
     * The line is drawn as a GL_QUADS.
     *
     * @param[in] beg The begin position of the line
     * @param[in] rot The direction the line shall be drawn to (in radiants)
     * @param[in] visLength The length of the line
     * @param[in] width The width of the line
     * @param[in] offset The orthogonal offset
     */
    static void drawBoxLine(const Position& beg, double rot,
                            double visLength, double width, double offset = 0);


    /** @brief Draws a thick line using the mean of both given points as begin position
     *
     * The line is drawn as a GL_QUADS.
     *
     * @param[in] beg1 One of the begin positions of the line to use the mean of
     * @param[in] beg2 One of the begin positions of the line to use the mean of
     * @param[in] rot The direction the line shall be drawn to (in radiants)
     * @param[in] visLength The length of the line
     * @param[in] width The width of the line
     */
    static void drawBoxLine(const Position& beg1, const Position& beg2,
                            double rot, double visLength, double width);


    /** @brief Draws thick lines
     *
     * Each line is drawn using drawBoxLine.
     *
     * @param[in] geom The list of begin positions of the lines
     * @param[in] rots The directions the lines shall be drawn to (in radiants)
     * @param[in] lengths The lengths of the lines
     * @param[in] width The width of the lines
     * @param[in] cornerDetail Detail level for filling the corners between angled segments
     * @param[in] the orthogonal offset
     * @see drawBoxLine
     */
    static void drawBoxLines(const PositionVector& geom,
                             const std::vector<double>& rots, const std::vector<double>& lengths,
                             double width, int cornerDetail = 0, double offset = 0);

    /** @brief Draws thick lines with varying color
     *
     * Each line is drawn using drawBoxLine.
     *
     * @param[in] geom The list of begin positions of the lines
     * @param[in] rots The directions the lines shall be drawn to (in radiants)
     * @param[in] lengths The lengths of the lines
     * @param[in] cols The colors of the lines
     * @param[in] width The width of the lines
     * @param[in] cornerDetail Detail level for filling the corners between angled segments
     * @param[in] the orthogonal offset
     * @see drawBoxLine
     */
    static void drawBoxLines(const PositionVector& geom,
                             const std::vector<double>& rots, const std::vector<double>& lengths,
                             const std::vector<RGBColor>& cols,
                             double width, int cornerDetail = 0, double offset = 0);


    /** @brief Draws thick lines using the mean of the points given in the point lists as begin positions
     *
     * Each line is drawn using drawBoxLine.
     *
     * @param[in] geom1 One of the lists to obtain the lines' begin positions to use the mean of
     * @param[in] geom2 One of the lists to obtain the lines' begin positions to use the mean of
     * @param[in] rots The directions the lines shall be drawn to (in radiants)
     * @param[in] lengths The lengths of the lines
     * @param[in] width The width of the lines
     * @see drawBoxLine
     */
    static void drawBoxLines(const PositionVector& geom1,
                             const PositionVector& geom2,
                             const std::vector<double>& rots, const std::vector<double>& lengths,
                             double width);


    /** @brief Draws thick lines
     *
     * Widths and length are computed internally by this method, each line is then
     *  drawn using drawBoxLine.
     *
     * @param[in] geom The list of begin positions of the lines
     * @param[in] width The width of the lines
     * @see drawBoxLine
     */
    static void drawBoxLines(const PositionVector& geom, double width);


    /** @brief Draws a thin line
     *
     * The line is drawn as a GL_LINES.
     *
     * @param[in] beg The begin position of the line
     * @param[in] rot The direction the line shall be drawn to (in radiants)
     * @param[in] visLength The length of the line
     */
    static void drawLine(const Position& beg, double rot,
                         double visLength);


    /** @brief Draws a thin line using the mean of both given points as begin position
     *
     * The line is drawn as a GL_LINES.
     *
     * @param[in] beg1 One of the begin positions of the line to use the mean of
     * @param[in] beg2 One of the begin positions of the line to use the mean of
     * @param[in] rot The direction the line shall be drawn to (in radiants)
     * @param[in] visLength The length of the line
     */
    static void drawLine(const Position& beg1, const Position& beg2,
                         double rot, double visLength);


    /** @brief Draws a thin line along the given position vector
     *
     * The line is drawn as a GL_LINES.
     *
     * @param[in] v The positions vector to use
     */
    static void drawLine(const PositionVector& v);


    /** @brief Draws a thin line along the given position vector with variable color
     *
     * The line is drawn as a GL_LINES.
     *
     * @param[in] v The positions vector to use
     * @param[in] cols vector with the color of every line
     * @note cols must have the same size as v
     */
    static void drawLine(const PositionVector& v, const std::vector<RGBColor>& cols);


    /** @brief Draws a thin line between the two points
     *
     * The line is drawn as a GL_LINES.
     *
     * @param[in] beg Begin of the line
     * @param[in] end End of the line
     */
    static void drawLine(const Position& beg, const Position& end);


    /** @brief Draws a filled circle around (0,0)
     *
     * The circle is drawn by calling drawFilledCircle(width, steps, 0, 360).
     *
     * @param[in] width The width of the circle
     * @param[in] steps The number of steps to divide the circle into
     */
    static void drawFilledCircle(double width, int steps = 8);


    /** @brief Draws a filled circle around (0,0)
     *
     * The circle is drawn use GL_TRIANGLES.
     *
     * @param[in] width The width of the circle
     * @param[in] steps The number of steps to divide the circle into
     * @param[in] beg The begin angle in degress
     * @param[in] end The end angle in degress
     */
    static void drawFilledCircle(double width, int steps,
                                 double beg, double end);


    /** @brief Draws an unfilled circle around (0,0)
     *
     * The circle is drawn by calling drawOutlineCircle(width, iwidth, steps, 0, 360).
     *
     * @param[in] width The (outer) width of the circle
     * @param[in] iwidth The inner width of the circle
     * @param[in] steps The number of steps to divide the circle into
     */
    static void drawOutlineCircle(double width, double iwidth,
                                  int steps = 8);


    /** @brief Draws an unfilled circle around (0,0)
     *
     * The circle is drawn use GL_TRIANGLES.
     *
     * @param[in] width The (outer) width of the circle
     * @param[in] iwidth The inner width of the circle
     * @param[in] steps The number of steps to divide the circle into
     * @param[in] beg The begin angle in degress
     * @param[in] end The end angle in degress
     */
    static void drawOutlineCircle(double width, double iwidth,
                                  int steps, double beg, double end);


    /** @brief Draws a triangle at the end of the given line
     *
     * @param[in] p1 The start of the line at which end the triangle shall be drawn
     * @param[in] p2 The end of the line at which end the triangle shall be drawn
     * @param[in] tLength The length of the triangle
     * @param[in] tWidth The width of the triangle
     */
    static void drawTriangleAtEnd(const Position& p1, const Position& p2,
                                  double tLength, double tWidth);

    /// @brief Sets the gl-color to this value
    static void setColor(const RGBColor& c);

    /// @brief gets the gl-color
    static RGBColor getColor();

    /* @brief draw Text with given parameters
     * when width is not given (negative) the font is scaled proportionally in
     * height and width according to size.
     *
     * align: see foreign/fontstash/fontstash.h for flags
    */
    static void drawText(const std::string& text, const Position& pos,
                         const double layer, const double size,
                         const RGBColor& col = RGBColor::BLACK,
                         const double angle = 0,
                         const int align = 0,
                         double width = -1);

    static void drawTextSettings(
        const GUIVisualizationTextSettings& settings,
        const std::string& text, const Position& pos,
        const double scale,
        const double angle = 0,
        const double layer = 2048, // GLO_MAX
        const int align = 0); // centered

    /// @brief draw Text box with given parameters
    static void drawTextBox(const std::string& text, const Position& pos,
                            const double layer, const double size,
                            const RGBColor& txtColor = RGBColor::BLACK,
                            const RGBColor& bgColor = RGBColor::WHITE,
                            const RGBColor& borderColor = RGBColor::BLACK,
                            const double angle = 0,
                            const double relBorder = 0.05,
                            const double relMargin = 0.5,
                            const int align = 0);

    /// @brief draw text and the end of shape
    static void drawTextAtEnd(const std::string& text, const PositionVector& shape, double x,
                              const GUIVisualizationTextSettings& settings, const double scale);

    /// @brief draw crossties for railroads or pedestrian crossings
    static void drawCrossTies(const PositionVector& geom,
                              const std::vector<double>& rots,
                              const std::vector<double>& lengths,
                              double length, double spacing, double halfWidth, bool drawForRectangleSelection);

    /// @brief draw vertex numbers for the given shape (in a random color)
    static void debugVertices(const PositionVector& shape, double size, double layer = 256);

    /// @brief Draw a boundary (used for debugging)
    static void drawBoundary(const Boundary& b);

    /// @brief to be called when the font context is invalidated
    static void resetFont();

    static void setGL2PS(bool active = true) {
        myGL2PSActive = active;
    }

private:
    /// @brief normalize angle for lookup in myCircleCoords
    static int angleLookup(double angleDeg);

    /// @brief whether the road makes a right turn (or goes straight)
    static bool rightTurn(double angle1, double angle2);

    /// @brief init myFont
    static bool initFont();

    /// @brief get dotted contour colors (black and white). Vector will be automatically increased if current size is minor than size
    static const std::vector<RGBColor>& getDottedcontourColors(const int size);

    /// @brief Storage for precomputed sin/cos-values describing a circle
    static std::vector<std::pair<double, double> > myCircleCoords;

    /// @brief Font context
    static struct FONScontext* myFont;
    static double myFontSize;

    /// @brief whether we are currently rendering for gl2ps
    static bool myGL2PSActive;

    /// @brief static vector with a list of alternated black/white colors (used for contourns)
    static std::vector<RGBColor> myDottedcontourColors;
};
