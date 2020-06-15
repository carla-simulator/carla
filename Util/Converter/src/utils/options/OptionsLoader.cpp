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
/// @file    OptionsLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
///
// A SAX-Handler for loading options
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <string>
#include <vector>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include "OptionsLoader.h"
#include "OptionsCont.h"
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>


// ===========================================================================
// method definitions
// ===========================================================================
OptionsLoader::OptionsLoader(const bool rootOnly)
    : myRootOnly(rootOnly), myError(false), myOptions(OptionsCont::getOptions()), myItem() {}


OptionsLoader::~OptionsLoader() {}


void OptionsLoader::startElement(const XMLCh* const name,
                                 XERCES_CPP_NAMESPACE::AttributeList& attributes) {
    myItem = StringUtils::transcode(name);
    if (!myRootOnly) {
        for (int i = 0; i < (int)attributes.getLength(); i++) {
            std::string key = StringUtils::transcode(attributes.getName(i));
            std::string value = StringUtils::transcode(attributes.getValue(i));
            if (key == "value" || key == "v") {
                // Substitute environment variables defined by ${NAME} with their value
                std::string cleanValue = StringUtils::substituteEnvironment(value);
                setValue(myItem, cleanValue);
            }
            // could give a hint here about unsupported attributes in configuration files
        }
        myValue = "";
    }
}


void OptionsLoader::setValue(const std::string& key,
                             std::string& value) {
    if (value.length() > 0) {
        try {
            if (!setSecure(key, value)) {
                WRITE_ERROR("Could not set option '" + key + "' (probably defined twice).");
                myError = true;
            }
        } catch (ProcessError& e) {
            WRITE_ERROR(e.what());
            myError = true;
        }
    }
}


void OptionsLoader::characters(const XMLCh* const chars,
                               const XERCES3_SIZE_t length) {
    myValue = myValue + StringUtils::transcode(chars, (int) length);
}


bool
OptionsLoader::setSecure(const std::string& name,
                         const std::string& value) const {
    if (myOptions.isWriteable(name)) {
        myOptions.set(name, value);
        return true;
    }
    return false;
}


void
OptionsLoader::endElement(const XMLCh* const /*name*/) {
    if (myItem.length() == 0 || myValue.length() == 0) {
        return;
    }
    if (myValue.find_first_not_of("\n\t \a") == std::string::npos) {
        return;
    }
    setValue(myItem, myValue);
    myItem = "";
    myValue = "";
}


void
OptionsLoader::warning(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    WRITE_WARNING(StringUtils::transcode(exception.getMessage()));
    WRITE_WARNING(" (At line/column " \
                  + toString(exception.getLineNumber() + 1) + '/' \
                  + toString(exception.getColumnNumber()) + ").");
    myError = true;
}


void
OptionsLoader::error(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    WRITE_ERROR(
        StringUtils::transcode(exception.getMessage()));
    WRITE_ERROR(
        " (At line/column "
        + toString(exception.getLineNumber() + 1) + '/'
        + toString(exception.getColumnNumber()) + ").");
    myError = true;
}


void
OptionsLoader::fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    WRITE_ERROR(
        StringUtils::transcode(exception.getMessage()));
    WRITE_ERROR(
        " (At line/column "
        + toString(exception.getLineNumber() + 1) + '/'
        + toString(exception.getColumnNumber()) + ").");
    myError = true;
}


bool
OptionsLoader::errorOccurred() const {
    return myError;
}


/****************************************************************************/
