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
/// @file    GUICompleteSchemeStorage.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    2006-01-09
///
// Storage for available visualization settings
/****************************************************************************/
#include <config.h>

#include "GUICompleteSchemeStorage.h"
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/RGBColor.h>
#include <utils/foxtools/MFXUtils.h>
#include <utils/gui/settings/GUISettingsHandler.h>
#include <utils/iodevices/OutputDevice_String.h>


// ===========================================================================
// static variable definitions
// ===========================================================================
GUICompleteSchemeStorage gSchemeStorage;


// ===========================================================================
// method definitions
// ===========================================================================
GUICompleteSchemeStorage::GUICompleteSchemeStorage() { }


GUICompleteSchemeStorage::~GUICompleteSchemeStorage() { }



void
GUICompleteSchemeStorage::add(const GUIVisualizationSettings& scheme) {
    std::string name = scheme.name;
    if (std::find(mySortedSchemeNames.begin(), mySortedSchemeNames.end(), name) == mySortedSchemeNames.end()) {
        mySortedSchemeNames.push_back(name);
    }
    mySettings[name] = scheme;
}


GUIVisualizationSettings&
GUICompleteSchemeStorage::get(const std::string& name) {
    return mySettings.find(name)->second;
}


GUIVisualizationSettings&
GUICompleteSchemeStorage::getDefault() {
    return mySettings.find(myDefaultSettingName)->second;
}


bool
GUICompleteSchemeStorage::contains(const std::string& name) const {
    return mySettings.find(name) != mySettings.end();
}


void
GUICompleteSchemeStorage::remove(const std::string& name) {
    if (!contains(name)) {
        return;
    }
    mySortedSchemeNames.erase(find(mySortedSchemeNames.begin(), mySortedSchemeNames.end(), name));
    mySettings.erase(mySettings.find(name));
}


void
GUICompleteSchemeStorage::setDefault(const std::string& name) {
    if (!contains(name)) {
        return;
    }
    myDefaultSettingName = name;
}


const std::vector<std::string>&
GUICompleteSchemeStorage::getNames() const {
    return mySortedSchemeNames;
}


int
GUICompleteSchemeStorage::getNumInitialSettings() const {
    return myNumInitialSettings;
}


void
GUICompleteSchemeStorage::init(FXApp* app, bool netedit) {
    {
        GUIVisualizationSettings vs(netedit);
        vs.name = "standard";
        vs.laneShowBorders = true;
        gSchemeStorage.add(vs);
    }
    {
        GUIVisualizationSettings vs(netedit);
        vs.name = "faster standard";
        vs.laneShowBorders = false;
        vs.showLinkDecals = false;
        vs.showRails = false;
        vs.showRails = false;
        vs.showSublanes = false;
        gSchemeStorage.add(vs);
    }
    {
        GUIVisualizationSettings vs(netedit);
        vs.name = "real world";
        vs.vehicleQuality = 2;
        vs.backgroundColor = RGBColor(51, 128, 51, 255);
        vs.laneShowBorders = true;
        vs.hideConnectors = true;
        vs.vehicleSize.minSize = 0;
        vs.personQuality = 2;
        vs.containerQuality = 2;
        vs.showSublanes = false;
        gSchemeStorage.add(vs);
    }
    {
        GUIVisualizationSettings vs(netedit);
        vs.name = "rail";
        vs.vehicleQuality = 2;
        vs.showLaneDirection = true;
        vs.spreadSuperposed = true;
        vs.junctionSize.constantSize = true;
        vs.junctionColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_TYPE);
        gSchemeStorage.add(vs);
    }

    if (!netedit) {
        GUIVisualizationSettings vs(netedit);
        vs.name = "selection";
        vs.vehicleColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.edgeColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.laneColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.junctionColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.personColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.containerColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.poiColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        vs.polyColorer.setSchemeByName(GUIVisualizationSettings::SCHEME_NAME_SELECTION);
        gSchemeStorage.add(vs);
    }
    myNumInitialSettings = (int) mySortedSchemeNames.size();
    // add saved settings
    int noSaved = app->reg().readIntEntry("VisualizationSettings", "settingNo", 0);
    for (int i = 0; i < noSaved; ++i) {
        std::string name = "visset#" + toString(i);
        std::string setting = app->reg().readStringEntry("VisualizationSettings", name.c_str(), "");
        if (setting != "") {
            GUIVisualizationSettings vs(netedit);

            vs.name = setting;
            app->reg().readStringEntry("VisualizationSettings", name.c_str(), "");

            // add saved xml setting
            int xmlSize = app->reg().readIntEntry(name.c_str(), "xmlSize", 0);
            std::string content = "";
            int index = 0;
            while (xmlSize > 0) {
                std::string part = app->reg().readStringEntry(name.c_str(), ("xml" + toString(index)).c_str(), "");
                if (part == "") {
                    break;
                }
                content += part;
                xmlSize -= (int) part.size();
                index++;
            }
            if (content != "" && xmlSize == 0) {
                try {
                    GUISettingsHandler handler(content, false, netedit);
                    handler.addSettings();
                } catch (ProcessError&) { }
            }
        }
    }
    myDefaultSettingName = mySortedSchemeNames[0];
    myLookFrom.set(0, 0, 0);
}


void
GUICompleteSchemeStorage::writeSettings(FXApp* app) {
    const std::vector<std::string>& names = getNames();
    app->reg().writeIntEntry("VisualizationSettings", "settingNo", (FXint) names.size() - myNumInitialSettings);
    int gidx = 0;
    for (std::vector<std::string>::const_iterator i = names.begin() + myNumInitialSettings; i != names.end(); ++i, ++gidx) {
        const GUIVisualizationSettings& item = mySettings.find(*i)->second;
        std::string sname = "visset#" + toString(gidx);

        app->reg().writeStringEntry("VisualizationSettings", sname.c_str(), item.name.c_str());
        OutputDevice_String dev;
        item.save(dev);
        std::string content = dev.getString();
        app->reg().writeIntEntry(sname.c_str(), "xmlSize", (FXint)(content.size()));
        const unsigned maxSize = 1500; // this is a fox limitation for registry entries
        for (int i = 0; i < (int)content.size(); i += maxSize) {
            const std::string b = content.substr(i, maxSize);
            app->reg().writeStringEntry(sname.c_str(), ("xml" + toString(i / maxSize)).c_str(), b.c_str());
        }
    }
}


void
GUICompleteSchemeStorage::saveViewport(const double x, const double y, const double z, const double rot) {
    myLookFrom.set(x, y, z);
    myRotation = rot;
}


void
GUICompleteSchemeStorage::setViewport(GUISUMOAbstractView* view) {
    if (myLookFrom.z() > 0) {
        // look straight down
        view->setViewportFromToRot(myLookFrom, Position(myLookFrom.x(), myLookFrom.y(), 0), myRotation);
    } else {
        view->recenterView();
    }
}


/****************************************************************************/
