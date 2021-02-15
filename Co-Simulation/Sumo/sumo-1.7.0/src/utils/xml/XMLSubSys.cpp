/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    XMLSubSys.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 1 Jul 2002
///
// Utility methods for initialising, closing and using the XML-subsystem
/****************************************************************************/
#include <config.h>

#include <cstdint>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include "SUMOSAXHandler.h"
#include "SUMOSAXReader.h"
#include "XMLSubSys.h"

using XERCES_CPP_NAMESPACE::SAX2XMLReader;
using XERCES_CPP_NAMESPACE::XMLPlatformUtils;
using XERCES_CPP_NAMESPACE::XMLReaderFactory;


// ===========================================================================
// static member variables
// ===========================================================================
std::vector<SUMOSAXReader*> XMLSubSys::myReaders;
int XMLSubSys::myNextFreeReader;
SAX2XMLReader::ValSchemes XMLSubSys::myValidationScheme = SAX2XMLReader::Val_Auto;
SAX2XMLReader::ValSchemes XMLSubSys::myNetValidationScheme = SAX2XMLReader::Val_Auto;
SAX2XMLReader::ValSchemes XMLSubSys::myRouteValidationScheme = SAX2XMLReader::Val_Auto;
XERCES_CPP_NAMESPACE::XMLGrammarPool* XMLSubSys::myGrammarPool = nullptr;


// ===========================================================================
// method definitions
// ===========================================================================
void
XMLSubSys::init() {
    try {
        XMLPlatformUtils::Initialize();
        myNextFreeReader = 0;
    } catch (const XERCES_CPP_NAMESPACE::XMLException& e) {
        throw ProcessError("Error during XML-initialization:\n " + StringUtils::transcode(e.getMessage()));
    }
}


void
XMLSubSys::setValidation(const std::string& validationScheme, const std::string& netValidationScheme, const std::string& routeValidationScheme) {
    if (validationScheme == "never") {
        myValidationScheme = SAX2XMLReader::Val_Never;
    } else if (validationScheme == "auto") {
        myValidationScheme = SAX2XMLReader::Val_Auto;
    } else if (validationScheme == "always") {
        myValidationScheme = SAX2XMLReader::Val_Always;
    } else {
        throw ProcessError("Unknown xml validation scheme + '" + validationScheme + "'.");
    }
    if (netValidationScheme == "never") {
        myNetValidationScheme = SAX2XMLReader::Val_Never;
    } else if (netValidationScheme == "auto") {
        myNetValidationScheme = SAX2XMLReader::Val_Auto;
    } else if (netValidationScheme == "always") {
        myNetValidationScheme = SAX2XMLReader::Val_Always;
    } else {
        throw ProcessError("Unknown network validation scheme + '" + netValidationScheme + "'.");
    }
    if (routeValidationScheme == "never") {
        myRouteValidationScheme = SAX2XMLReader::Val_Never;
    } else if (routeValidationScheme == "auto") {
        myRouteValidationScheme = SAX2XMLReader::Val_Auto;
    } else if (routeValidationScheme == "always") {
        myRouteValidationScheme = SAX2XMLReader::Val_Always;
    } else {
        throw ProcessError("Unknown route validation scheme + '" + routeValidationScheme + "'.");
    }
    if (myGrammarPool == nullptr &&
            (myValidationScheme != SAX2XMLReader::Val_Never ||
             myNetValidationScheme != SAX2XMLReader::Val_Never ||
             myRouteValidationScheme != SAX2XMLReader::Val_Never)) {
        myGrammarPool = new XERCES_CPP_NAMESPACE::XMLGrammarPoolImpl(XMLPlatformUtils::fgMemoryManager);
        SAX2XMLReader* parser(XMLReaderFactory::createXMLReader(XMLPlatformUtils::fgMemoryManager, myGrammarPool));
#if _XERCES_VERSION >= 30100
        parser->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgXercesHandleMultipleImports, true);
#endif
        const char* sumoPath = std::getenv("SUMO_HOME");
        if (sumoPath == nullptr) {
            WRITE_WARNING("Environment variable SUMO_HOME is not set, schema resolution will use slow website lookups.");
            return;
        }
        for (const std::string& filetype : {
                    "additional", "routes", "net"
                }) {
            const std::string file = sumoPath + std::string("/data/xsd/") + filetype + "_file.xsd";
            if (!parser->loadGrammar(file.c_str(), XERCES_CPP_NAMESPACE::Grammar::SchemaGrammarType, true)) {
                WRITE_WARNING("Cannot read local schema '" + file + "', will try website lookup.");
            }
        }
    }
}


void
XMLSubSys::close() {
    for (std::vector<SUMOSAXReader*>::iterator i = myReaders.begin(); i != myReaders.end(); ++i) {
        delete *i;
    }
    myReaders.clear();
    delete myGrammarPool;
    myGrammarPool = nullptr;
    XMLPlatformUtils::Terminate();
}


SUMOSAXReader*
XMLSubSys::getSAXReader(SUMOSAXHandler& handler, const bool isNet, const bool isRoute) {
    SAX2XMLReader::ValSchemes validationScheme = isNet ? myNetValidationScheme : myValidationScheme;
    if (isRoute) {
        validationScheme = myRouteValidationScheme;
    }
    return new SUMOSAXReader(handler, validationScheme, myGrammarPool);
}


void
XMLSubSys::setHandler(GenericSAXHandler& handler) {
    myReaders[myNextFreeReader - 1]->setHandler(handler);
}


bool
XMLSubSys::runParser(GenericSAXHandler& handler, const std::string& file,
                     const bool isNet, const bool isRoute) {
    MsgHandler::getErrorInstance()->clear();
    try {
        SAX2XMLReader::ValSchemes validationScheme = isNet ? myNetValidationScheme : myValidationScheme;
        if (isRoute) {
            validationScheme = myRouteValidationScheme;
        }
        if (myNextFreeReader == (int)myReaders.size()) {
            myReaders.push_back(new SUMOSAXReader(handler, validationScheme, myGrammarPool));
        } else {
            myReaders[myNextFreeReader]->setValidation(validationScheme);
            myReaders[myNextFreeReader]->setHandler(handler);
        }
        myNextFreeReader++;
        std::string prevFile = handler.getFileName();
        handler.setFileName(file);
        myReaders[myNextFreeReader - 1]->parse(file);
        handler.setFileName(prevFile);
        myNextFreeReader--;
    } catch (AbortParsing&) {
        return false;
    } catch (ProcessError& e) {
        WRITE_ERROR(std::string(e.what()) != std::string("") ? std::string(e.what()) : std::string("Process Error"));
        return false;
    } catch (const std::runtime_error& re) {
        WRITE_ERROR("Runtime error: " + std::string(re.what()) + " while parsing '" + file + "'");
        return false;
    } catch (const std::exception& ex) {
        WRITE_ERROR("Error occurred: " + std::string(ex.what()) + " while parsing '" + file + "'");
        return false;
    } catch (...) {
        WRITE_ERROR("Unspecified error occurred wile parsing '" + file + "'");
        return false;
    }
    return !MsgHandler::getErrorInstance()->wasInformed();
}


/****************************************************************************/
