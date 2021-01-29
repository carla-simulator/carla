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
/// @file    GUIPropertyScheme.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Mon, 20.07.2009
///
//
/****************************************************************************/
#pragma once
#include <config.h>

#include <cassert>
#include <vector>
#include <utils/common/RGBColor.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/gui/images/GUIIcons.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIPropertyScheme
 * This class provides a mapping from real values to properties (mainly colors).
 * Each color is stored along with a threshold value.
 * Color values between thresholds are obtained by interpolation
 */

template<class T>
class GUIPropertyScheme {
public:
    /// Constructor
    GUIPropertyScheme(const std::string& name, const T& baseColor,
                      const std::string& colName = "", const bool isFixed = false, double baseValue = 0,
                      RGBColor bgColor = RGBColor::INVISIBLE,
                      GUIIcon icon = GUIIcon::EMPTY) :
        myName(name), myIsInterpolated(!isFixed),
        myIsFixed(isFixed),
        myAllowNegativeValues(false),
        myIcon(icon),
        myBgColor(bgColor) {
        addColor(baseColor, baseValue, colName);
    }

    void setThreshold(const int pos, const double threshold) {
        myThresholds[pos] = threshold;
    }

    void setColor(const int pos, const T& color) {
        myColors[pos] = color;
    }

    bool setColor(const std::string& name, const T& color) {
        std::vector<std::string>::iterator nameIt = myNames.begin();
        typename std::vector<T>::iterator colIt = myColors.begin();
        for (; nameIt != myNames.end(); ++nameIt, ++colIt) {
            if (*nameIt == name) {
                (*colIt) = color;
                return true;
            }
        }
        return false;
    }

    int addColor(const T& color, const double threshold, const std::string& name = "") {
        typename std::vector<T>::iterator colIt = myColors.begin();
        std::vector<double>::iterator threshIt = myThresholds.begin();
        std::vector<std::string>::iterator nameIt = myNames.begin();
        int pos = 0;
        while (threshIt != myThresholds.end() && (*threshIt) < threshold) {
            ++threshIt;
            ++colIt;
            ++nameIt;
            pos++;
        }
        myColors.insert(colIt, color);
        myThresholds.insert(threshIt, threshold);
        myNames.insert(nameIt, name);
        return pos;
    }

    void removeColor(const int pos) {
        assert(pos < (int)myColors.size());
        myColors.erase(myColors.begin() + pos);
        myThresholds.erase(myThresholds.begin() + pos);
        myNames.erase(myNames.begin() + pos);
    }

    void clear() {
        myColors.clear();
        myThresholds.clear();
        myNames.clear();
    }

    const T getColor(const double value) const {
        if (myColors.size() == 1 || value < myThresholds.front()) {
            return myColors.front();
        }
        typename std::vector<T>::const_iterator colIt = myColors.begin() + 1;
        std::vector<double>::const_iterator threshIt = myThresholds.begin() + 1;
        while (threshIt != myThresholds.end() && (*threshIt) <= value) {
            ++threshIt;
            ++colIt;
        }
        if (threshIt == myThresholds.end()) {
            return myColors.back();
        }
        if (!myIsInterpolated) {
            return *(colIt - 1);
        }
        double lowVal = *(threshIt - 1);
        return interpolate(*(colIt - 1), *colIt, (value - lowVal) / ((*threshIt) - lowVal));
    }

    void setInterpolated(const bool interpolate, double interpolationStart = 0.f) {
        myIsInterpolated = interpolate;
        if (interpolate) {
            myThresholds[0] = interpolationStart;
        }
    }

    const std::string& getName() const {
        return myName;
    }

    const std::vector<T>& getColors() const {
        return myColors;
    }

    const std::vector<double>& getThresholds() const {
        return myThresholds;
    }

    bool isInterpolated() const {
        return myIsInterpolated;
    }

    const std::vector<std::string>& getNames() const {
        return myNames;
    }

    bool isFixed() const {
        return myIsFixed;
    }

    bool allowsNegativeValues() const {
        return myAllowNegativeValues;
    }

    void setAllowsNegativeValues(bool value) {
        myAllowNegativeValues = value;
    }

    GUIIcon getIcon() const {
        return myIcon;
    }

    const RGBColor& getBackgroundColor() const {
        return myBgColor;
    }

    void save(OutputDevice& dev) const {
        const std::string tag = getTagName(myColors);

        dev.openTag(tag);
        dev.writeAttr(SUMO_ATTR_NAME, myName);
        if (!myIsFixed) {
            dev.writeAttr(SUMO_ATTR_INTERPOLATED, myIsInterpolated);
        }
        typename std::vector<T>::const_iterator colIt = myColors.begin();
        std::vector<double>::const_iterator threshIt = myThresholds.begin();
        std::vector<std::string>::const_iterator nameIt = myNames.begin();
        while (threshIt != myThresholds.end()) {
            dev.openTag(SUMO_TAG_ENTRY);
            dev.writeAttr(SUMO_ATTR_COLOR, *colIt);
            if (!myIsFixed && (*threshIt) != std::numeric_limits<double>::max()) {
                dev.writeAttr(SUMO_ATTR_THRESHOLD, *threshIt);
            }
            if ((*nameIt) != "") {
                dev.writeAttr(SUMO_ATTR_NAME, *nameIt);
            }
            dev.closeTag();
            ++threshIt;
            ++colIt;
            ++nameIt;
        }
        dev.closeTag();
    }

    bool operator==(const GUIPropertyScheme& c) const {
        return myName == c.myName && myColors == c.myColors && myThresholds == c.myThresholds && myIsInterpolated == c.myIsInterpolated;
    }


    /// @brief specializations for GUIColorScheme
    RGBColor interpolate(const RGBColor& min, const RGBColor& max, double weight) const {
        return RGBColor::interpolate(min, max, weight);
    }

    std::string getTagName(std::vector<RGBColor>) const {
        return toString(SUMO_TAG_COLORSCHEME);
    }


    /// @brief specializations for GUIScaleScheme
    double interpolate(const double& min, const double& max, double weight) const {
        return min + (max - min) * weight;
    }

    std::string getTagName(std::vector<double>) const {
        return toString(SUMO_TAG_SCALINGSCHEME);
    }


private:
    std::string myName;
    std::vector<T> myColors;
    std::vector<double> myThresholds;
    bool myIsInterpolated;
    std::vector<std::string> myNames;
    bool myIsFixed;
    bool myAllowNegativeValues;
    GUIIcon myIcon;
    RGBColor myBgColor;

};

typedef GUIPropertyScheme<RGBColor> GUIColorScheme;
typedef GUIPropertyScheme<double> GUIScaleScheme;
