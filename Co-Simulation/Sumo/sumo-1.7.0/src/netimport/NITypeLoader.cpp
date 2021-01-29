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
/// @file    NITypeLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Robert Hilbrich
/// @date    Tue, 20 Nov 2001
///
// Perfoms network import
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOSAXReader.h>
#include <utils/xml/XMLSubSys.h>
#include "NITypeLoader.h"


bool
NITypeLoader::load(SUMOSAXHandler* handler, const std::vector<std::string>& files,
                   const std::string& type, const bool stringParse) {
    // build parser
    std::string exceptMsg = "";
    std::string fileName = "";
    // start the parsing
    bool ok = true;
    bool raise = false;
    try {
        for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
            fileName = *file;
            if (stringParse) {
                fileName = "built in type map";
                handler->setFileName(fileName);
                SUMOSAXReader* reader = XMLSubSys::getSAXReader(*handler);
                reader->parseString(*file);
                delete reader;
                continue;
            }
            if (!FileHelpers::isReadable(fileName)) {
                WRITE_ERROR("Could not open " + type + "-file '" + fileName + "'.");
                return false;
            }
            PROGRESS_BEGIN_MESSAGE("Parsing " + type + " from '" + fileName + "'");
            ok &= XMLSubSys::runParser(*handler, fileName);
            PROGRESS_DONE_MESSAGE();
        }
    } catch (const XERCES_CPP_NAMESPACE::XMLException& toCatch) {
        exceptMsg = StringUtils::transcode(toCatch.getMessage()) + "\n  ";
        raise = true;
    } catch (const ProcessError& toCatch) {
        exceptMsg = std::string(toCatch.what()) + "\n  ";
        raise = true;
    } catch (...) {
        raise = true;
    }
    delete handler;
    if (raise) {
        throw ProcessError(exceptMsg + "The " + type + " could not be loaded from '" + fileName + "'.");
    }
    return ok;
}


/****************************************************************************/
