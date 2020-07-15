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
/// @file    OptionsParser.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
///
// Parses the command line arguments
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <cstring>
#include "Option.h"
#include "OptionsCont.h"
#include "OptionsParser.h"
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>


// ===========================================================================
// method definitions
// ===========================================================================
bool
OptionsParser::parse(int argc, char** argv) {
    bool ok = true;
    for (int i = 1; i < argc;) {
        try {
            int add;
            // try to set the current option
            if (i < argc - 1) {
                add = check(argv[i], argv[i + 1], ok);
            } else {
                add = check(argv[i], nullptr, ok);
            }
            i += add;
        } catch (ProcessError& e) {
            WRITE_ERROR("On processing option '" + std::string(argv[i]) + "':\n " + e.what());
            i++;
            ok = false;
        }
    }
    return ok;
}

bool
OptionsParser::parse(std::vector<std::string> args) {
    bool ok = true;
    for (size_t i = 1; i < args.size();) {
        try {
            int add;
            // try to set the current option
            if (i < args.size()- 1) {
                add = check(args[i].c_str(), args[i + 1].c_str(), ok);
            } else {
                add = check(args[i].c_str(), nullptr, ok);
            }
            i += add;
        } catch (ProcessError& e) {
            WRITE_ERROR("On processing option '" + args[i] + "':\n " + e.what());
            i++;
            ok = false;
        }
    }
    return ok;
}

int
OptionsParser::check(const char* arg1, const char* arg2, bool& ok) {
    // the first argument should be an option
    // (only the second may be a free string)
    if (!checkParameter(arg1)) {
        ok = false;
        return 1;
    }

    OptionsCont& oc = OptionsCont::getOptions();
    // process not abbreviated switches
    if (!isAbbreviation(arg1)) {
        std::string tmp(arg1 + 2);
        const std::string::size_type idx1 = tmp.find('=');
        // check whether a parameter was submitted
        if (idx1 != std::string::npos) {
            ok &= oc.set(tmp.substr(0, idx1), tmp.substr(idx1 + 1));
        } else {
            if (arg2 == nullptr || (oc.isBool(convert(arg1 + 2)) && arg2[0] == '-')) {
                ok &= oc.set(convert(arg1 + 2), "true");
            } else {
                ok &= oc.set(convert(arg1 + 2), convert(arg2));
                return 2;
            }
        }
        return 1;
    }
    // go through the abbreviated switches
    for (int i = 1; arg1[i] != 0; i++) {
        // set boolean switches
        if (oc.isBool(convert(arg1[i]))) {
            if (arg2 == nullptr || arg2[0] == '-' || arg1[i + 1] != 0) {
                ok &= oc.set(convert(arg1[i]), "true");
            } else {
                ok &= oc.set(convert(arg1[i]), convert(arg2));
                return 2;
            }
            // set non-boolean switches
        } else {
            // check whether the parameter comes directly after the switch
            //  and process if so
            if (arg2 == nullptr || arg1[i + 1] != 0) {
                ok &= processNonBooleanSingleSwitch(oc, arg1 + i);
                return 1;
                // process parameter following after a space
            } else {
                ok &= oc.set(convert(arg1[i]), convert(arg2));
                // option name and attribute were in two arguments
                return 2;
            }
        }
    }
    // all switches within the current argument were boolean switches
    return 1;
}


bool
OptionsParser::processNonBooleanSingleSwitch(OptionsCont& oc, const char* arg) {
    if (arg[1] == '=') {
        if (strlen(arg) < 3) {
            WRITE_ERROR("Missing value for parameter '" + std::string(arg).substr(0, 1) + "'.");
            return false;
        } else {
            return oc.set(convert(arg[0]), std::string(arg + 2));
        }
    } else {
        if (strlen(arg) < 2) {
            WRITE_ERROR("Missing value for parameter '" + std::string(arg) + "'.");
            return false;
        } else {
            return oc.set(convert(arg[0]), std::string(arg + 1));
        }
    }
}


bool
OptionsParser::checkParameter(const char* arg1) {
    if (arg1[0] != '-') {
        WRITE_ERROR("The parameter '" + std::string(arg1) + "' is not allowed in this context.\n Switch or parameter name expected.");
        return false;
    }
    return true;
}


bool
OptionsParser::isAbbreviation(const char* arg1) {
    return arg1[1] != '-';
}


std::string
OptionsParser::convert(const char* arg) {
    std::string s(arg);
    return s;
}


std::string
OptionsParser::convert(char abbr) {
    char buf[2];
    buf[0] = abbr;
    buf[1] = 0;
    std::string s(buf);
    return buf;
}


/****************************************************************************/
