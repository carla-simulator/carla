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
/// @file    GUISelectedStorage.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Jun 2004
///
// Storage for "selected" objects
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include "GUISelectedStorage.h"
#include "GUIDialog_GLChosenEditor.h"
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/ToString.h>


// ===========================================================================
// member method definitions
// ===========================================================================

/* -------------------------------------------------------------------------
 * for GUISelectedStorage::SingleTypeSelections
 * ----------------------------------------------------------------------- */

GUISelectedStorage::SingleTypeSelections::SingleTypeSelections() {}


GUISelectedStorage::SingleTypeSelections::~SingleTypeSelections() {}


bool
GUISelectedStorage::SingleTypeSelections::isSelected(GUIGlID id) {
    return mySelected.count(id) > 0;
}


void
GUISelectedStorage::SingleTypeSelections::select(GUIGlID id) {
    mySelected.insert(id);
}


void
GUISelectedStorage::SingleTypeSelections::deselect(GUIGlID id) {
    mySelected.erase(id);
}


void
GUISelectedStorage::SingleTypeSelections::clear() {
    mySelected.clear();
}


void
GUISelectedStorage::SingleTypeSelections::save(const std::string& filename) {
    GUISelectedStorage::save(filename, mySelected);
}


const std::set<GUIGlID>&
GUISelectedStorage::SingleTypeSelections::getSelected() const {
    return mySelected;
}

/* -------------------------------------------------------------------------
 * for GUISelectedStorage
 * ----------------------------------------------------------------------- */

GUISelectedStorage::GUISelectedStorage() {}


GUISelectedStorage::~GUISelectedStorage() {}


bool
GUISelectedStorage::isSelected(GUIGlObjectType type, GUIGlID id) {
    switch (type) {
        case GLO_NETWORK:
            return false;
        default:
            return mySelections[type].isSelected(id);
    }
}

bool
GUISelectedStorage::isSelected(const GUIGlObject* o) {
    if (o == nullptr) {
        return false;
    } else {
        return isSelected(o->getType(), o->getGlID());
    }
}

void
GUISelectedStorage::select(GUIGlID id, bool update) {
    GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
    if (!object) {
        throw ProcessError("Unkown object in GUISelectedStorage::select (id=" + toString(id) + ").");
    }
    GUIGlObjectType type = object->getType();
    GUIGlObjectStorage::gIDStorage.unblockObject(id);

    mySelections[type].select(id);
    myAllSelected.insert(id);
    if (update && myUpdateTarget) {
        myUpdateTarget->selectionUpdated();
    }
}


void
GUISelectedStorage::deselect(GUIGlID id) {
    GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
    if (!object) {
        throw ProcessError("Unkown object in GUISelectedStorage::deselect (id=" + toString(id) + ").");
    }
    GUIGlObjectType type = object->getType();
    GUIGlObjectStorage::gIDStorage.unblockObject(id);

    mySelections[type].deselect(id);
    myAllSelected.erase(id);
    if (myUpdateTarget) {
        myUpdateTarget->selectionUpdated();
    }
}


void
GUISelectedStorage::toggleSelection(GUIGlID id) {
    GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
    if (!object) {
        throw ProcessError("Unkown object in GUISelectedStorage::toggleSelection (id=" + toString(id) + ").");
    }

    bool selected = isSelected(object->getType(), id);
    if (!selected) {
        select(id);
    } else {
        deselect(id);
    }
    GUIGlObjectStorage::gIDStorage.unblockObject(id);
}


const std::set<GUIGlID>&
GUISelectedStorage::getSelected() const {
    return myAllSelected;
}


const std::set<GUIGlID>&
GUISelectedStorage::getSelected(GUIGlObjectType type) {
    return mySelections[type].getSelected();
}


void
GUISelectedStorage::clear() {
    for (std::map<GUIGlObjectType, SingleTypeSelections>::iterator it = mySelections.begin(); it != mySelections.end(); it++) {
        it->second.clear();
    }
    myAllSelected.clear();
    if (myUpdateTarget) {
        myUpdateTarget->selectionUpdated();
    }
}


std::set<GUIGlID>
GUISelectedStorage::loadIDs(const std::string& filename, std::string& msgOut, GUIGlObjectType type, int maxErrors) {
    std::set<GUIGlID> result;
    std::ostringstream msg;
    std::ifstream strm(filename.c_str());
    int numIgnored = 0;
    int numMissing = 0;
    if (!strm.good()) {
        msgOut = "Could not open '" + filename + "'.\n";
        return result;
    }
    while (strm.good()) {
        std::string line;
        strm >> line;
        if (line.length() == 0) {
            continue;
        }

        GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(line);
        if (object) {
            if (type != GLO_MAX && (object->getType() != type)) {
                numIgnored++;
                if (numIgnored + numMissing <= maxErrors) {
                    msg << "Ignoring item '" << line << "' because of invalid type " << toString(object->getType()) << "\n";
                }
            } else {
                result.insert(object->getGlID());
            }
        } else {
            numMissing++;
            if (numIgnored + numMissing <= maxErrors) {
                msg << "Item '" + line + "' not found\n";
            }
            continue;
        }
    }
    strm.close();
    if (numIgnored + numMissing > maxErrors) {
        msg << "...\n" << numIgnored << " objects ignored, " << numMissing << " objects not found\n";
    }
    msgOut = msg.str();
    return result;
}


std::string
GUISelectedStorage::load(const std::string& filename, GUIGlObjectType type) {
    std::string errors;
    const std::set<GUIGlID> ids = loadIDs(filename, errors, type);
    for (std::set<GUIGlID>::const_iterator it = ids.begin(); it != ids.end(); it++) {
        select(*it, false);
    }
    if (myUpdateTarget) {
        myUpdateTarget->selectionUpdated();
    }
    return errors;
}


void
GUISelectedStorage::save(GUIGlObjectType type, const std::string& filename) {
    mySelections[type].save(filename);
}


void
GUISelectedStorage::save(const std::string& filename) const {
    save(filename, myAllSelected);
}


void
GUISelectedStorage::add2Update(UpdateTarget* updateTarget) {
    myUpdateTarget = updateTarget;
}


void
GUISelectedStorage::remove2Update() {
    myUpdateTarget = nullptr;
}


void
GUISelectedStorage::save(const std::string& filename, const std::set<GUIGlID>& ids) {
    OutputDevice& dev = OutputDevice::getDevice(filename);
    for (std::set<GUIGlID>::const_iterator i = ids.begin(); i != ids.end(); ++i) {
        GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(*i);
        if (object != nullptr) {
            std::string name = object->getFullName();
            dev << name << "\n";
            GUIGlObjectStorage::gIDStorage.unblockObject(*i);
        }
    }
    dev.close();
}


/****************************************************************************/
