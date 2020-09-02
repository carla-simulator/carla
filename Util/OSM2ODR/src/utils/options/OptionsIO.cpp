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
/// @file    OptionsIO.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
///
// Helper for parsing command line arguments and reading configuration files
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include "OptionsIO.h"
#include "OptionsCont.h"
#include "OptionsLoader.h"
#include "OptionsParser.h"
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#ifdef HAVE_ZLIB
#include <foreign/zstr/zstr.hpp>
#endif
#include <utils/xml/IStreamInputSource.h>


// ===========================================================================
// static member definitions
// ===========================================================================
int OptionsIO::myArgC = 0;
char** OptionsIO::myArgV;


// ===========================================================================
// method definitions
// ===========================================================================
void
OptionsIO::setArgs(int argc, char** argv) {
    myArgC = argc;
    myArgV = argv;
}


void
OptionsIO::setArgs(const std::vector<std::string>& args) {
    char* const app = myArgC > 0 ? myArgV[0] : nullptr;
    myArgC = (int)args.size() + 1;
    char** argv = new char* [myArgC];
    argv[0] = app;
    for (int i = 1; i < myArgC; i++) {
        argv[i] = new char[args[i - 1].size() + 1];
        std::strcpy(argv[i], args[i - 1].c_str());
    }
    myArgV = argv;
}


void
OptionsIO::getOptions(const bool commandLineOnly) {
    if (myArgC == 2 && myArgV[1][0] != '-') {
        // special case only one parameter, check who can handle it
        if (OptionsCont::getOptions().setByRootElement(getRoot(myArgV[1]), myArgV[1])) {
            if (!commandLineOnly) {
                loadConfiguration();
            }
            return;
        }
    }
    // preparse the options
    //  (maybe another configuration file was chosen)
    if (!OptionsParser::parse(myArgC, myArgV)) {
        throw ProcessError("Could not parse commandline options.");
    }
    if (!commandLineOnly || OptionsCont::getOptions().isSet("save-configuration", false)) {
        // read the configuration when everything's ok
        loadConfiguration();
    }
}


void
OptionsIO::loadConfiguration() {
    OptionsCont& oc = OptionsCont::getOptions();
    if (!oc.exists("configuration-file") || !oc.isSet("configuration-file")) {
        return;
    }
    const std::string path = oc.getString("configuration-file");
    if (!FileHelpers::isReadable(path)) {
        throw ProcessError("Could not access configuration '" + oc.getString("configuration-file") + "'.");
    }
    PROGRESS_BEGIN_MESSAGE("Loading configuration");
    oc.resetWritable();
    // build parser
    XERCES_CPP_NAMESPACE::SAXParser parser;
    parser.setValidationScheme(XERCES_CPP_NAMESPACE::SAXParser::Val_Auto);
    parser.setDoNamespaces(false);
    parser.setDoSchema(false);
    // start the parsing
    OptionsLoader handler;
    try {
        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);
        parser.parse(path.c_str());
        if (handler.errorOccurred()) {
            throw ProcessError("Could not load configuration '" + path + "'.");
        }
    } catch (const XERCES_CPP_NAMESPACE::XMLException& e) {
        throw ProcessError("Could not load configuration '" + path + "':\n " + StringUtils::transcode(e.getMessage()));
    }
    oc.relocateFiles(path);
    if (myArgC > 2) {
        // reparse the options (overwrite the settings from the configuration file)
        oc.resetWritable();
        OptionsParser::parse(myArgC, myArgV);
    }
    PROGRESS_DONE_MESSAGE();
}


std::string
OptionsIO::getRoot(const std::string& filename) {
    // build parser
    XERCES_CPP_NAMESPACE::SAXParser parser;
    // start the parsing
    OptionsLoader handler;
    try {
        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);
        XERCES_CPP_NAMESPACE::XMLPScanToken token;
#ifdef HAVE_ZLIB
        zstr::ifstream istream(filename.c_str(), std::fstream::in | std::fstream::binary);
        IStreamInputSource inputStream(istream);
        const bool result = parser.parseFirst(inputStream, token);
#else
        const bool result = parser.parseFirst(filename.c_str(), token);
#endif
        if (!result) {
            throw ProcessError("Can not read XML-file '" + filename + "'.");
        }
        while (parser.parseNext(token) && handler.getItem() == "");
        if (handler.errorOccurred()) {
            throw ProcessError("Could not load '" + filename + "'.");
        }
    } catch (const XERCES_CPP_NAMESPACE::XMLException& e) {
        throw ProcessError("Could not load '" + filename + "':\n " + StringUtils::transcode(e.getMessage()));
    }
    return handler.getItem();
}


/****************************************************************************/
