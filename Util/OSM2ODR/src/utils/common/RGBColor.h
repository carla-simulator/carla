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
/// @file    RGBColor.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A RGB-color definition
/****************************************************************************/
#pragma once
#include <iostream>
#include <random>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RGBColor
 * The definition of a color in the RGB-space with an alpha channel.
 * The cube is meant to lie between (0, 0, 0) and (255, 255, 255)
 */
class RGBColor {
public:
    /** @brief Constructor
     */
    RGBColor();

    /** @brief Constructor
     * @param[in] red The red component's value
     * @param[in] green The green component's value
     * @param[in] blue The blue component's value
     */
    RGBColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255);

    /// @brief Copy constructor
    RGBColor(const RGBColor& col);

    /// @brief Destructor
    ~RGBColor();

    /** @brief Returns the red-amount of the color
     * @return The red component's value
     */
    unsigned char red() const {
        return myRed;
    }

    /** @brief Returns the green-amount of the color
     * @return The green component's value
     */
    unsigned char green() const {
        return myGreen;
    }

    /** @brief Returns the blue-amount of the color
     * @return The blue component's value
     */
    unsigned char blue() const {
        return myBlue;
    }

    /** @brief Returns the alpha-amount of the color
     * @return The alpha component's value
     */
    unsigned char alpha() const {
        return myAlpha;
    }

    /** @brief assigns new values
     * @param[in] r The red component's value
     * @param[in] g The green component's value
     * @param[in] b The blue component's value
     * @param[in] a The alpha component's value
     */
    void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    /** @brief Sets a new alpha value
     * @param[in] alpha The new value to use
     */
    inline void setAlpha(unsigned char alpha) {
        myAlpha = alpha;
    }


    /** @brief Returns a new color with altered brightness
     * @param[in] change The absolute change applied to all channels (within bounds)
     * @param[in] change The number of colors to change
     * @return The new color
     */
    RGBColor changedBrightness(int change, int toChange = 3) const;

    /// @brief obtain inverted of current RGBColor
    RGBColor invertedColor() const;

    static std::mt19937* getColorRNG() {
        return &myRNG;
    }

    /** @brief Parses a color information
     *
     * It is assumed that the color is stored as "<RED>,<GREEN>,<BLUE>"
     *  and each color is represented as a double.
     *  Alternatively the color can be stored as "<RED>,<GREEN>,<BLUE>,<ALPHA>"
     *  and each color is represented as an unsigned byte.
     * @param[in] coldef The color definition to parse
     * @return The parsed color
     * @exception EmptyData If the definition has less than three entries
     * @exception NumberFormatException If one of the components is not numeric
     */
    static RGBColor parseColor(std::string coldef);

    /** @brief Parses a color information
     *
     * It is assumed that the color is stored as "<RED>,<GREEN>,<BLUE>"
     *  and each color is represented as a double.
     *  Alternatively the color can be stored as "<RED>,<GREEN>,<BLUE>,<ALPHA>"
     *  and each color is represented as an unsigned byte.
     * @param[in] coldef The color definition to parse
     * @param[in] objecttype The type of the currently parsed object
     * @param[in] objectid The id of the currently parsed object
     * @param[in] report Whether errors shall be reported
     * @param[in, out] ok Whether parsing was successful
     * @return The parsed color
     * @exception EmptyData If the definition has less than three entries
     * @exception NumberFormatException If one of the components is not numeric
     */
    static RGBColor parseColorReporting(const std::string& coldef, const std::string& objecttype,
                                        const char* objectid, bool report, bool& ok);

    /** @brief Interpolates between two colors
     *
     * The interpolated color is calculated as a weighted average of
     *  the RGB values of minColor and maxColor, giving weight to maxColor
     *  and 1-weight to minColor.
     * @param[in] minColor The color to interpolate from
     * @param[in] maxColor The color to interpolate to
     * @param[in] weight The weight of the first color
     * @return The interpolated color
     */
    static RGBColor interpolate(const RGBColor& minColor, const RGBColor& maxColor, double weight);

    /** @brief Converts the given hsv-triplet to rgb, inspired by http://alvyray.com/Papers/CG/hsv2rgb.htm
     * @param[in] h Hue (0-360)
     * @param[in] s Saturation (0-1)
     * @param[in] v Value (0-1)
     * @return The color as RGB
     */
    static RGBColor fromHSV(double h, double s, double v);

    /** @brief Return color with random hue
     * @param[in] s Saturation (0-1)
     * @param[in] v Value (0-1)
     * @return The color as RGB
     */
    static RGBColor randomHue(double s = 1, double v = 1);

    /** @brief Writes the color to the given stream
    * @param[out] os The stream to write to
    * @param[in] col The color to write
    * @return The stream
    */
    friend std::ostream& operator<<(std::ostream& os, const RGBColor& col);

    // @brief Equality operator
    bool operator==(const RGBColor& c) const;

    // @brief Inequality operator
    bool operator!=(const RGBColor& c) const;

    /// @brief named colors
    /// @{
    static const RGBColor RED;
    static const RGBColor GREEN;
    static const RGBColor BLUE;
    static const RGBColor YELLOW;
    static const RGBColor CYAN;
    static const RGBColor MAGENTA;
    static const RGBColor ORANGE;
    static const RGBColor WHITE;
    static const RGBColor BLACK;
    static const RGBColor GREY;
    static const RGBColor INVISIBLE;
    /// @}

    /// @brief The default color (for vehicle types and vehicles)
    static const RGBColor DEFAULT_COLOR;

    /// @brief The string description of the default color
    static const std::string DEFAULT_COLOR_STRING;

private:
    /// @brief The color amounts
    unsigned char myRed, myGreen, myBlue, myAlpha;

    /// @brief A random number generator to generate random colors independent of other randomness
    static std::mt19937 myRNG;
};
