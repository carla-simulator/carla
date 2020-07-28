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
/// @file    SUMOVehicleClass.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @author  Laura Bieker-Walz
/// @date    2006-01-24
///
// Definitions of SUMO vehicle classes and helper functions
/****************************************************************************/
#include <config.h>

#include <string>
#include <map>
#include "SUMOVehicleClass.h"
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/xml/SUMOSAXAttributes.h>


// ===========================================================================
// static members
// ===========================================================================

StringBijection<SUMOVehicleClass>::Entry sumoVehicleClassStringInitializer[] = {
    {"ignoring",          SVC_IGNORING},
    {"private",           SVC_PRIVATE},
    {"public_emergency",  SVC_EMERGENCY}, // !!! deprecated
    {"emergency",         SVC_EMERGENCY},
    {"public_authority",  SVC_AUTHORITY}, // !!! deprecated
    {"authority",         SVC_AUTHORITY},
    {"public_army",       SVC_ARMY}, // !!! deprecated
    {"army",              SVC_ARMY},
    {"vip",               SVC_VIP},
    {"passenger",         SVC_PASSENGER},
    {"hov",               SVC_HOV},
    {"taxi",              SVC_TAXI},
    {"public_transport",  SVC_BUS}, // !!! deprecated
    {"bus",               SVC_BUS},
    {"coach",             SVC_COACH},
    {"delivery",          SVC_DELIVERY},
    {"transport",         SVC_TRUCK},
    {"truck",             SVC_TRUCK},
    {"trailer",           SVC_TRAILER},
    {"lightrail",         SVC_TRAM}, // !!! deprecated
    {"tram",              SVC_TRAM},
    {"cityrail",          SVC_RAIL_URBAN}, // !!! deprecated
    {"rail_urban",        SVC_RAIL_URBAN},
    {"rail_slow",         SVC_RAIL}, // !!! deprecated
    {"rail",              SVC_RAIL},
    {"rail_fast",         SVC_RAIL_FAST},
    {"rail_electric",     SVC_RAIL_ELECTRIC},
    {"motorcycle",        SVC_MOTORCYCLE},
    {"moped",             SVC_MOPED},
    {"bicycle",           SVC_BICYCLE},
    {"pedestrian",        SVC_PEDESTRIAN},
    {"evehicle",          SVC_E_VEHICLE},
    {"ship",              SVC_SHIP},
    {"custom1",           SVC_CUSTOM1},
    {"custom2",           SVC_CUSTOM2}
};

StringBijection<SUMOVehicleClass> SumoVehicleClassStrings(
    sumoVehicleClassStringInitializer, SVC_CUSTOM2, false);


std::set<std::string> deprecatedVehicleClassesSeen;


StringBijection<SUMOVehicleShape>::Entry sumoVehicleShapeStringInitializer[] = {
    {"pedestrian",            SVS_PEDESTRIAN},
    {"bicycle",               SVS_BICYCLE},
    {"moped",                 SVS_MOPED},
    {"motorcycle",            SVS_MOTORCYCLE},
    {"passenger",             SVS_PASSENGER},
    {"passenger/sedan",       SVS_PASSENGER_SEDAN},
    {"passenger/hatchback",   SVS_PASSENGER_HATCHBACK},
    {"passenger/wagon",       SVS_PASSENGER_WAGON},
    {"passenger/van",         SVS_PASSENGER_VAN},
    {"delivery",              SVS_DELIVERY},
    {"transport",             SVS_TRUCK}, // !!! deprecated
    {"truck",                 SVS_TRUCK},
    {"transport/semitrailer", SVS_TRUCK_SEMITRAILER}, // !!! deprecated
    {"truck/semitrailer",     SVS_TRUCK_SEMITRAILER},
    {"transport/trailer",     SVS_TRUCK_1TRAILER}, // !!! deprecated
    {"truck/trailer",         SVS_TRUCK_1TRAILER},
    {"bus/city",              SVS_BUS}, // !!! deprecated
    {"bus",                   SVS_BUS},
    {"bus/overland",          SVS_BUS_COACH}, // !!! deprecated
    {"bus/coach",             SVS_BUS_COACH},
    {"bus/flexible",          SVS_BUS_FLEXIBLE},
    {"bus/trolley",           SVS_BUS_TROLLEY},
    {"rail/slow",             SVS_RAIL}, // !!! deprecated
    {"rail/fast",             SVS_RAIL}, // !!! deprecated
    {"rail",                  SVS_RAIL},
    {"rail/light",            SVS_RAIL_CAR}, // !!! deprecated
    {"rail/city",             SVS_RAIL_CAR}, // !!! deprecated
    {"rail/railcar",          SVS_RAIL_CAR},
    {"rail/cargo",            SVS_RAIL_CARGO},
    {"evehicle",              SVS_E_VEHICLE},
    {"ant",                   SVS_ANT},
    {"ship",                  SVS_SHIP},
    {"emergency",             SVS_EMERGENCY},
    {"firebrigade",           SVS_FIREBRIGADE},
    {"police",                SVS_POLICE},
    {"rickshaw",              SVS_RICKSHAW },
    {"",                      SVS_UNKNOWN}
};


StringBijection<SUMOVehicleShape> SumoVehicleShapeStrings(
    sumoVehicleShapeStringInitializer, SVS_UNKNOWN, false);

// ===========================================================================
// static values used for cached
// ===========================================================================

static std::map<int, std::vector<std::string> > vehicleClassNamesListCached;
static std::map<std::string, SVCPermissions> parseVehicleClassesCached;
static std::map<SVCPermissions, std::string> getVehicleClassNamesCached;
static std::string vehicleClassNameAll = "all";

// ===========================================================================
// additional constants
// ===========================================================================

const SUMOVehicleClass SUMOVehicleClass_MAX = SVC_CUSTOM2;

const SVCPermissions SVCAll = 2 * (int)SUMOVehicleClass_MAX - 1; // all relevant bits set to 1

const SVCPermissions SVC_UNSPECIFIED = -1;

const std::string DEFAULT_VTYPE_ID("DEFAULT_VEHTYPE");

const std::string DEFAULT_PEDTYPE_ID("DEFAULT_PEDTYPE");

const std::string DEFAULT_BIKETYPE_ID("DEFAULT_BIKETYPE");

const std::string DEFAULT_CONTAINERTYPE_ID("DEFAULT_CONTAINERTYPE");

const std::string DEFAULT_TAXITYPE_ID("DEFAULT_TAXITYPE");

const double DEFAULT_VEH_PROB(1.);

const double DEFAULT_PEDESTRIAN_SPEED(5. / 3.6);

const double DEFAULT_CONTAINER_TRANSHIP_SPEED(5. / 3.6);

// ===========================================================================
// method definitions
// ===========================================================================
// ------------ Conversion of SUMOVehicleClass

const std::string&
getVehicleClassNames(SVCPermissions permissions, bool expand) {
    if (permissions == SVCAll && !expand) {
        return vehicleClassNameAll;
    }
    // check if previously was cached
    if (getVehicleClassNamesCached.count(permissions) == 0) {
        getVehicleClassNamesCached[permissions] = joinToString(getVehicleClassNamesList(permissions), ' ');
    }
    return getVehicleClassNamesCached.at(permissions);
}


const std::vector<std::string>&
getVehicleClassNamesList(SVCPermissions permissions) {
    // first check if it's cached
    if (vehicleClassNamesListCached.count(permissions) == 0) {
        const std::vector<std::string> classNames = SumoVehicleClassStrings.getStrings();
        std::vector<std::string> result;
        for (std::vector<std::string>::const_iterator it = classNames.begin(); it != classNames.end(); it++) {
            const int svc = (int)SumoVehicleClassStrings.get(*it);
            if ((svc & permissions) == svc && svc != SVC_IGNORING) {
                result.push_back(*it);
            }
        }
        // add it into vehicleClassNamesListCached
        vehicleClassNamesListCached[permissions] = result;
    }
    return vehicleClassNamesListCached.at(permissions);
}


SUMOVehicleClass
getVehicleClassID(const std::string& name) {
    if (SumoVehicleClassStrings.hasString(name)) {
        return SumoVehicleClassStrings.get(name);
    }
    throw InvalidArgument("Unknown vehicle class '" + name + "'.");
}


int
getVehicleClassCompoundID(const std::string& name) {
    int ret = SVC_IGNORING;
    const std::vector<std::string> names = SumoVehicleClassStrings.getStrings();
    for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); it++) {
        if (name.find(*it) != std::string::npos) {
            ret = ret | (int) SumoVehicleClassStrings.get(*it);
        }
    }
    return ret;
}


SVCPermissions
parseVehicleClasses(const std::string& allowedS) {
    if (allowedS == "all") {
        return SVCAll;
    }
    // check  if allowedS was previously cached
    if (parseVehicleClassesCached.count(allowedS) == 0) {
        SVCPermissions result = 0;
        StringTokenizer sta(allowedS, " ");
        while (sta.hasNext()) {
            const std::string s = sta.next();
            if (!SumoVehicleClassStrings.hasString(s)) {
                WRITE_ERROR("Unknown vehicle class '" + s + "' encountered.");
            } else {
                const SUMOVehicleClass vc = getVehicleClassID(s);
                const std::string& realName = SumoVehicleClassStrings.getString(vc);
                if (realName != s) {
                    deprecatedVehicleClassesSeen.insert(s);
                }
                result |= vc;
            }
        }
        // save parsed vehicle class cached
        parseVehicleClassesCached[allowedS] = result;
    }
    return parseVehicleClassesCached.at(allowedS);
}


bool
canParseVehicleClasses(const std::string& classes) {
    if (classes == "all") {
        return true;
    }
    // check if was previously cached
    if (parseVehicleClassesCached.count(classes) != 0) {
        return true;
    }
    StringTokenizer sta(classes, " ");
    while (sta.hasNext()) {
        if (!SumoVehicleClassStrings.hasString(sta.next())) {
            return false;
        }
    }
    return true;
}


SVCPermissions
parseVehicleClasses(const std::string& allowedS, const std::string& disallowedS, double networkVersion) {
    if (allowedS.size() == 0 && disallowedS.size() == 0) {
        return SVCAll;
    } else if (allowedS.size() > 0 && disallowedS.size() > 0) {
        WRITE_WARNING("SVCPermissions must be specified either via 'allow' or 'disallow'. Ignoring 'disallow'");
        return parseVehicleClasses(allowedS);
    } else if (allowedS.size() > 0) {
        return parseVehicleClasses(allowedS);
    } else {
        return invertPermissions(parseVehicleClasses(disallowedS) | (networkVersion < 1.3 ? SVC_RAIL_FAST : 0));
    }
}


SVCPermissions
invertPermissions(SVCPermissions permissions) {
    return SVCAll & ~permissions;
}


SVCPermissions
parseVehicleClasses(const std::vector<std::string>& allowedS) {
    SVCPermissions result = 0;
    if (std::find(allowedS.begin(), allowedS.end(), "all") != allowedS.end()) {
        return SVCAll;
    }
    for (std::vector<std::string>::const_iterator i = allowedS.begin(); i != allowedS.end(); ++i) {
        const SUMOVehicleClass vc = getVehicleClassID(*i);
        const std::string& realName = SumoVehicleClassStrings.getString(vc);
        if (realName != *i) {
            WRITE_WARNING("The vehicle class '" + (*i) + "' is deprecated, use '" + realName + "' instead.");
        }
        result |= getVehicleClassID(*i);
    }
    return result;
}


void
writePermissions(OutputDevice& into, SVCPermissions permissions) {
    if (permissions == SVCAll) {
        return;
    } else if (permissions == 0) {
        into.writeAttr(SUMO_ATTR_DISALLOW, "all");
        return;
    } else {
        int num_allowed = 0;
        for (int mask = 1; mask <= SUMOVehicleClass_MAX; mask = mask << 1) {
            if ((mask & permissions) == mask) {
                ++num_allowed;
            }
        }
        if (num_allowed <= (SumoVehicleClassStrings.size() - num_allowed) && num_allowed > 0) {
            into.writeAttr(SUMO_ATTR_ALLOW, getVehicleClassNames(permissions));
        } else {
            into.writeAttr(SUMO_ATTR_DISALLOW, getVehicleClassNames(~permissions));
        }
    }
}


void
writePreferences(OutputDevice& into, SVCPermissions preferred) {
    if (preferred == SVCAll || preferred == 0) {
        return;
    } else {
        into.writeAttr(SUMO_ATTR_PREFER, getVehicleClassNames(preferred));
    }
}


SUMOVehicleShape
getVehicleShapeID(const std::string& name) {
    if (SumoVehicleShapeStrings.hasString(name)) {
        return SumoVehicleShapeStrings.get(name);
    } else {
        throw InvalidArgument("Unknown vehicle shape '" + name + "'.");
    }
}


bool
canParseVehicleShape(const std::string& shape) {
    return SumoVehicleShapeStrings.hasString(shape);
}


std::string
getVehicleShapeName(SUMOVehicleShape id) {
    return SumoVehicleShapeStrings.getString(id);
}


bool isRailway(SVCPermissions permissions) {
    return (permissions & SVC_RAIL_CLASSES) > 0 && (permissions & SVC_PASSENGER) == 0;
}


bool
isWaterway(SVCPermissions permissions) {
    return permissions == SVC_SHIP;
}


bool
isForbidden(SVCPermissions permissions) {
    return (permissions & SVCAll) == 0;
}


bool
isSidewalk(SVCPermissions permissions) {
    return (permissions & SVCAll) == SVC_PEDESTRIAN;
}


bool
noVehicles(SVCPermissions permissions) {
    return isForbidden(permissions) || isSidewalk(permissions);
}


std::map<SVCPermissions, double> parseStopOffsets(const SUMOSAXAttributes& attrs, bool& ok) {
    const std::string vClasses = attrs.getOpt<std::string>(SUMO_ATTR_VCLASSES, nullptr, ok, "");
    const std::string exceptions = attrs.getOpt<std::string>(SUMO_ATTR_EXCEPTIONS, nullptr, ok, "");
    if (attrs.hasAttribute(SUMO_ATTR_VCLASSES) && attrs.hasAttribute(SUMO_ATTR_EXCEPTIONS)) {
        WRITE_ERROR("Simultaneous specification of vClasses and exceptions is not allowed!");
        ok = false;
        return std::map<SVCPermissions, double>();
    }
    const double value = attrs.get<double>(SUMO_ATTR_VALUE, nullptr, ok);

    int vClassBitset;
    if (attrs.hasAttribute(SUMO_ATTR_VCLASSES)) {
        vClassBitset = parseVehicleClasses(vClasses);
    } else if (attrs.hasAttribute(SUMO_ATTR_EXCEPTIONS)) {
        vClassBitset = ~parseVehicleClasses(exceptions);
    } else {
        // no vClasses specified, thus apply to all
        vClassBitset = parseVehicleClasses("all");
    }

    std::map<SVCPermissions, double> offsets;
    offsets[vClassBitset] = value;
    return offsets;
}


double
getDefaultVehicleLength(const SUMOVehicleClass vc) {
    switch (vc) {
        case SVC_PEDESTRIAN:
            return 0.215;
        case SVC_BICYCLE:
            return 1.6;
        case SVC_MOPED:
            return 2.1;
        case SVC_MOTORCYCLE:
            return 2.2;
        case SVC_TRUCK:
            return 7.1;
        case SVC_TRAILER:
            return 16.5;
        case SVC_BUS:
            return 12.;
        case SVC_COACH:
            return 14.;
        case SVC_TRAM:
            return 22.;
        case SVC_RAIL_URBAN:
            return 36.5 * 3;
        case SVC_RAIL:
            return 67.5 * 2;
        case SVC_RAIL_ELECTRIC:
        case SVC_RAIL_FAST:
            return 25. * 8;
        case SVC_DELIVERY:
        case SVC_EMERGENCY:
            return 6.5;
        case SVC_SHIP:
            return 17;
        default:
            return 5; /*4.3*/
    }
}



/****************************************************************************/
