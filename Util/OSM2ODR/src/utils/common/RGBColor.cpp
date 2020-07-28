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
/// @file    RGBColor.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// A RGB-color definition
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <utils/common/RandHelper.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include "RGBColor.h"


// ===========================================================================
// static member definitions
// ===========================================================================
const RGBColor RGBColor::RED = RGBColor(255, 0, 0, 255);
const RGBColor RGBColor::GREEN = RGBColor(0, 255, 0, 255);
const RGBColor RGBColor::BLUE = RGBColor(0, 0, 255, 255);
const RGBColor RGBColor::YELLOW = RGBColor(255, 255, 0, 255);
const RGBColor RGBColor::CYAN = RGBColor(0, 255, 255, 255);
const RGBColor RGBColor::MAGENTA = RGBColor(255, 0, 255, 255);
const RGBColor RGBColor::ORANGE = RGBColor(255, 128, 0, 255);
const RGBColor RGBColor::WHITE = RGBColor(255, 255, 255, 255);
const RGBColor RGBColor::BLACK = RGBColor(0, 0, 0, 255);
const RGBColor RGBColor::GREY = RGBColor(128, 128, 128, 255);
const RGBColor RGBColor::INVISIBLE = RGBColor(0, 0, 0, 0);

const RGBColor RGBColor::DEFAULT_COLOR = RGBColor::YELLOW;
const std::string RGBColor::DEFAULT_COLOR_STRING = toString(RGBColor::DEFAULT_COLOR);

// random colors do not affect the simulation. No initialization is necessary
std::mt19937 RGBColor::myRNG;

// ===========================================================================
// method definitions
// ===========================================================================
RGBColor::RGBColor()
    : myRed(0), myGreen(0), myBlue(0), myAlpha(0) {}


RGBColor::RGBColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
    : myRed(red), myGreen(green), myBlue(blue), myAlpha(alpha) {}


RGBColor::RGBColor(const RGBColor& col)
    : myRed(col.myRed), myGreen(col.myGreen), myBlue(col.myBlue), myAlpha(col.myAlpha) {}


RGBColor::~RGBColor() {}


void
RGBColor::set(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    myRed = r;
    myGreen = g;
    myBlue = b;
    myAlpha = a;
}


std::ostream&
operator<<(std::ostream& os, const RGBColor& col) {
    if (col == RGBColor::RED) {
        return os << "red";
    }
    if (col == RGBColor::GREEN) {
        return os << "green";
    }
    if (col == RGBColor::BLUE) {
        return os << "blue";
    }
    if (col == RGBColor::YELLOW) {
        return os << "yellow";
    }
    if (col == RGBColor::CYAN) {
        return os << "cyan";
    }
    if (col == RGBColor::MAGENTA) {
        return os << "magenta";
    }
    if (col == RGBColor::ORANGE) {
        return os << "orange";
    }
    if (col == RGBColor::WHITE) {
        return os << "white";
    }
    if (col == RGBColor::BLACK) {
        return os << "black";
    }
    if (col == RGBColor::GREY) {
        return os << "grey";
    }
    os << static_cast<int>(col.myRed) << ","
       << static_cast<int>(col.myGreen) << ","
       << static_cast<int>(col.myBlue);
    if (col.myAlpha < 255) {
        os << "," << static_cast<int>(col.myAlpha);
    }
    return os;
}


bool
RGBColor::operator==(const RGBColor& c) const {
    return myRed == c.myRed && myGreen == c.myGreen && myBlue == c.myBlue && myAlpha == c.myAlpha;
}


bool
RGBColor::operator!=(const RGBColor& c) const {
    return myRed != c.myRed || myGreen != c.myGreen || myBlue != c.myBlue || myAlpha != c.myAlpha;
}


RGBColor
RGBColor::invertedColor() const {
    // obtain inverse colors
    const unsigned char r = (unsigned char)(255 - (int)myRed);
    const unsigned char g = (unsigned char)(255 - (int)myGreen);
    const unsigned char b = (unsigned char)(255  - (int)myBlue);
    // return inverted RBColor
    return RGBColor(r, g, b, myAlpha);
}


RGBColor
RGBColor::changedBrightness(int change, int toChange) const {
    const unsigned char red = (unsigned char)(MIN2(MAX2(myRed + change, 0), 255));
    const unsigned char blue = (unsigned char)(MIN2(MAX2(myBlue + change, 0), 255));
    const unsigned char green = (unsigned char)(MIN2(MAX2(myGreen + change, 0), 255));
    int changed = ((int)red - (int)myRed) + ((int)blue - (int)myBlue) + ((int)green - (int)myGreen);
    const RGBColor result(red, green, blue, myAlpha);
    if (changed == toChange * change) {
        return result;
    } else if (changed == 0) {
        return result;
    } else {
        const int maxedColors = (red != myRed + change ? 1 : 0) + (blue != myBlue + change ? 1 : 0) + (green != myGreen + change ? 1 : 0);
        if (maxedColors == 3) {
            return result;
        } else {
            const int toChangeNext = 3 - maxedColors;
            return result.changedBrightness((int)((toChange * change - changed) / toChangeNext), toChangeNext);
        }
    }
}


RGBColor
RGBColor::parseColor(std::string coldef) {
    std::transform(coldef.begin(), coldef.end(), coldef.begin(), tolower);
    if (coldef == "red") {
        return RED;
    }
    if (coldef == "green") {
        return GREEN;
    }
    if (coldef == "blue") {
        return BLUE;
    }
    if (coldef == "yellow") {
        return YELLOW;
    }
    if (coldef == "cyan") {
        return CYAN;
    }
    if (coldef == "magenta") {
        return MAGENTA;
    }
    if (coldef == "orange") {
        return ORANGE;
    }
    if (coldef == "white") {
        return WHITE;
    }
    if (coldef == "black") {
        return BLACK;
    }
    if (coldef == "grey" || coldef == "gray") {
        return GREY;
    }
    if (coldef == "invisible") {
        return INVISIBLE;
    }
    if (coldef == "random") {
        return fromHSV(RandHelper::rand(360, &myRNG),
                // prefer more saturated colors
                pow(RandHelper::rand(&myRNG), 0.3),
                // prefer brighter colors
                pow(RandHelper::rand(&myRNG), 0.3));
    }
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    unsigned char a = 255;
    if (coldef[0] == '#') {
        const int coldesc = StringUtils::hexToInt(coldef);
        if (coldef.length() == 7) {
            r = static_cast<unsigned char>((coldesc & 0xFF0000) >> 16);
            g = static_cast<unsigned char>((coldesc & 0x00FF00) >> 8);
            b = coldesc & 0xFF;
        } else if (coldef.length() == 9) {
            r = static_cast<unsigned char>((coldesc & 0xFF000000) >> 24);
            g = static_cast<unsigned char>((coldesc & 0x00FF0000) >> 16);
            b = static_cast<unsigned char>((coldesc & 0x0000FF00) >> 8);
            a = coldesc & 0xFF;
        } else {
            throw EmptyData();
        }
    } else {
        std::vector<std::string> st = StringTokenizer(coldef, ",").getVector();
        if (st.size() == 3 || st.size() == 4) {
            try {
                r = static_cast<unsigned char>(StringUtils::toInt(st[0]));
                g = static_cast<unsigned char>(StringUtils::toInt(st[1]));
                b = static_cast<unsigned char>(StringUtils::toInt(st[2]));
                if (st.size() == 4) {
                    a = static_cast<unsigned char>(StringUtils::toInt(st[3]));
                }
                if (r <= 1 && g <= 1 && b <= 1 && (st.size() == 3 || a <= 1)) {
                    throw NumberFormatException("(color component) " + coldef);
                }
            } catch (NumberFormatException&) {
                r = static_cast<unsigned char>(StringUtils::toDouble(st[0]) * 255. + 0.5);
                g = static_cast<unsigned char>(StringUtils::toDouble(st[1]) * 255. + 0.5);
                b = static_cast<unsigned char>(StringUtils::toDouble(st[2]) * 255. + 0.5);
                if (st.size() == 4) {
                    a = static_cast<unsigned char>(StringUtils::toDouble(st[3]) * 255. + 0.5);
                }
            }
        } else {
            throw EmptyData();
        }
    }
    return RGBColor(r, g, b, a);
}


RGBColor
RGBColor::parseColorReporting(
    const std::string& coldef, const std::string& objecttype,
    const char* objectid, bool report, bool& ok) {
    UNUSED_PARAMETER(report);
    try {
        return parseColor(coldef);
    } catch (NumberFormatException&) {
    } catch (EmptyData&) {
    }
    ok = false;
    std::ostringstream oss;
    oss << "Attribute 'color' in definition of ";
    if (objectid == nullptr) {
        oss << "a ";
    }
    oss << objecttype;
    if (objectid != nullptr) {
        oss << " '" << objectid << "'";
    }
    oss << " is not a valid color.";
    WRITE_ERROR(oss.str());
    return RGBColor();
}


RGBColor
RGBColor::interpolate(const RGBColor& minColor, const RGBColor& maxColor, double weight) {
    if (weight < 0) {
        weight = 0;
    }
    if (weight > 1) {
        weight = 1;
    }
    const unsigned char r = (unsigned char)((int)minColor.myRed   + (((int)maxColor.myRed   - (int)minColor.myRed)   * weight));
    const unsigned char g = (unsigned char)((int)minColor.myGreen + (((int)maxColor.myGreen - (int)minColor.myGreen) * weight));
    const unsigned char b = (unsigned char)((int)minColor.myBlue  + (((int)maxColor.myBlue  - (int)minColor.myBlue)  * weight));
    const unsigned char a = (unsigned char)((int)minColor.myAlpha + (((int)maxColor.myAlpha - (int)minColor.myAlpha) * weight));
    return RGBColor(r, g, b, a);
}


RGBColor
RGBColor::fromHSV(double h, double s, double v) {
    h /= 60.;
    const int i = int(floor(h));
    double f = h - i;
    if (i % 2 == 0) {
        f = 1. - f;
    }
    const unsigned char m = static_cast<unsigned char>(v * (1 - s) * 255. + 0.5);
    const unsigned char n = static_cast<unsigned char>(v * (1 - s * f) * 255. + 0.5);
    const unsigned char vv = static_cast<unsigned char>(v * 255. + 0.5);
    switch (i) {
        case 6:
        case 0:
            return RGBColor(vv, n, m, 255);
        case 1:
            return RGBColor(n, vv, m, 255);
        case 2:
            return RGBColor(m, vv, n, 255);
        case 3:
            return RGBColor(m, n, vv, 255);
        case 4:
            return RGBColor(n, m, vv, 255);
        case 5:
            return RGBColor(vv, m, n, 255);
    }
    return RGBColor(255, 255, 255, 255);
}

RGBColor
RGBColor::randomHue(double s, double v) {
    return fromHSV(RandHelper::rand(360, &myRNG), s, v);
}


/****************************************************************************/
