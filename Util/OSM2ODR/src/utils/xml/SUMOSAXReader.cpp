/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOSAXReader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Nov 2012
///
// SAX-reader encapsulation containing binary reader
/****************************************************************************/
#include <config.h>

#include <string>
#include <memory>
#include <iostream>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include "GenericSAXHandler.h"
#ifdef HAVE_ZLIB
#include <foreign/zstr/zstr.hpp>
#endif
#include "IStreamInputSource.h"
#include "SUMOSAXReader.h"


// ===========================================================================
// method definitions
// ===========================================================================
SUMOSAXReader::SUMOSAXReader(GenericSAXHandler& handler, const XERCES_CPP_NAMESPACE::SAX2XMLReader::ValSchemes validationScheme)
    : myHandler(nullptr), myValidationScheme(validationScheme), myXMLReader(nullptr), myIStream(nullptr), myInputStream(nullptr) {
    setHandler(handler);
}


SUMOSAXReader::~SUMOSAXReader() {
    delete myXMLReader;
}


void
SUMOSAXReader::setHandler(GenericSAXHandler& handler) {
    myHandler = &handler;
    mySchemaResolver.setHandler(handler);
    if (myXMLReader != nullptr) {
        myXMLReader->setContentHandler(&handler);
        myXMLReader->setErrorHandler(&handler);
    }
}


void
SUMOSAXReader::setValidation(const XERCES_CPP_NAMESPACE::SAX2XMLReader::ValSchemes validationScheme) {
    if (myXMLReader != nullptr && validationScheme != myValidationScheme) {
        if (validationScheme == XERCES_CPP_NAMESPACE::SAX2XMLReader::Val_Never) {
            myXMLReader->setEntityResolver(nullptr);
            myXMLReader->setProperty(XERCES_CPP_NAMESPACE::XMLUni::fgXercesScannerName, (void*)XERCES_CPP_NAMESPACE::XMLUni::fgWFXMLScanner);
        } else {
            myXMLReader->setEntityResolver(&mySchemaResolver);
            myXMLReader->setProperty(XERCES_CPP_NAMESPACE::XMLUni::fgXercesScannerName, (void*)XERCES_CPP_NAMESPACE::XMLUni::fgIGXMLScanner);
            myXMLReader->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgXercesSchema, true);
            myXMLReader->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgSAX2CoreValidation, true);
            myXMLReader->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgXercesDynamic, validationScheme == XERCES_CPP_NAMESPACE::SAX2XMLReader::Val_Auto);
        }
    }
    myValidationScheme = validationScheme;
}


void
SUMOSAXReader::parse(std::string systemID) {
    if (myXMLReader == nullptr) {
        myXMLReader = getSAXReader();
    }
    if (!FileHelpers::isReadable(systemID)) {
        throw ProcessError("Cannot read file '" + systemID + "'!");
    }
    if (FileHelpers::isDirectory(systemID)) {
        throw ProcessError("File '" + systemID + "' is a directory!");
    }
#ifdef HAVE_ZLIB
    zstr::ifstream istream(systemID.c_str(), std::fstream::in | std::fstream::binary);
    myXMLReader->parse(IStreamInputSource(istream));
#else
    myXMLReader->parse(systemID.c_str());
#endif
}


void
SUMOSAXReader::parseString(std::string content) {
    if (myXMLReader == nullptr) {
        myXMLReader = getSAXReader();
    }
    XERCES_CPP_NAMESPACE::MemBufInputSource memBufIS((const XMLByte*)content.c_str(), content.size(), "registrySettings");
    myXMLReader->parse(memBufIS);
}


bool
SUMOSAXReader::parseFirst(std::string systemID) {
    if (!FileHelpers::isReadable(systemID)) {
        throw ProcessError("Cannot read file '" + systemID + "'!");
    }
    if (myXMLReader == nullptr) {
        myXMLReader = getSAXReader();
    }
    myToken = XERCES_CPP_NAMESPACE::XMLPScanToken();
#ifdef HAVE_ZLIB
    myIStream = std::unique_ptr<zstr::ifstream>(new zstr::ifstream(systemID.c_str(), std::fstream::in | std::fstream::binary));
    myInputStream = std::unique_ptr<IStreamInputSource>(new IStreamInputSource(*myIStream));
    return myXMLReader->parseFirst(*myInputStream, myToken);
#else
    return myXMLReader->parseFirst(systemID.c_str(), myToken);
#endif
}


bool
SUMOSAXReader::parseNext() {
    if (myXMLReader == nullptr) {
        throw ProcessError("The XML-parser was not initialized.");
    }
    return myXMLReader->parseNext(myToken);
}


XERCES_CPP_NAMESPACE::SAX2XMLReader*
SUMOSAXReader::getSAXReader() {
    XERCES_CPP_NAMESPACE::SAX2XMLReader* reader = XERCES_CPP_NAMESPACE::XMLReaderFactory::createXMLReader();
    if (reader == nullptr) {
        throw ProcessError("The XML-parser could not be build.");
    }
    // see here https://svn.apache.org/repos/asf/xerces/c/trunk/samples/src/SAX2Count/SAX2Count.cpp for the way to set features
    if (myValidationScheme == XERCES_CPP_NAMESPACE::SAX2XMLReader::Val_Never) {
        reader->setProperty(XERCES_CPP_NAMESPACE::XMLUni::fgXercesScannerName, (void*)XERCES_CPP_NAMESPACE::XMLUni::fgWFXMLScanner);
    } else {
        reader->setEntityResolver(&mySchemaResolver);
        reader->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgXercesSchema, true);
        reader->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgSAX2CoreValidation, true);
        reader->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgXercesDynamic, myValidationScheme == XERCES_CPP_NAMESPACE::SAX2XMLReader::Val_Auto);
    }
    reader->setContentHandler(myHandler);
    reader->setErrorHandler(myHandler);
    return reader;
}


XERCES_CPP_NAMESPACE::InputSource*
SUMOSAXReader::LocalSchemaResolver::resolveEntity(const XMLCh* const /* publicId */, const XMLCh* const systemId) {
    const std::string url = StringUtils::transcode(systemId);
    const std::string::size_type pos = url.find("/xsd/");
    if (pos != std::string::npos) {
        myHandler->setSchemaSeen();
        // const char* sumoPath = std::getenv("SUMO_HOME");
        // if (sumoPath == nullptr) {
        //     WRITE_WARNING("Environment variable SUMO_HOME is not set, schema resolution will use slow website lookups.");
        //     return nullptr;
        // }
        // const std::string file = sumoPath + std::string("/data") + url.substr(pos);
        // if (FileHelpers::isReadable(file)) {
        //     XMLCh* t = XERCES_CPP_NAMESPACE::XMLString::transcode(file.c_str());
        //     XERCES_CPP_NAMESPACE::InputSource* const result = new XERCES_CPP_NAMESPACE::LocalFileInputSource(t);
        //     XERCES_CPP_NAMESPACE::XMLString::release(&t);
        //     return result;
        // } else {
        //     WRITE_WARNING("Cannot read local schema '" + file + "', will try website lookup.");
        // }
        const std::string file = std::string("../carla/data") + url.substr(pos);
        if (FileHelpers::isReadable(file)) {
            XMLCh* t = XERCES_CPP_NAMESPACE::XMLString::transcode(file.c_str());
            XERCES_CPP_NAMESPACE::InputSource* const result = new XERCES_CPP_NAMESPACE::LocalFileInputSource(t);
            XERCES_CPP_NAMESPACE::XMLString::release(&t);
            return result;
        } else {
            WRITE_WARNING("Cannot read local schema '" + file + "', will try website lookup.");
        }
    }
    return nullptr;
}


void
SUMOSAXReader::LocalSchemaResolver::setHandler(GenericSAXHandler& handler) {
    myHandler = &handler;
}


/****************************************************************************/
