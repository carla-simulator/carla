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
/// @file    GenericSAXHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// A handler which converts occuring elements and attributes into enums
/****************************************************************************/
#include <config.h>

#include <cassert>
#include "GenericSAXHandler.h"
#include <utils/common/StringUtils.h>
#include <utils/common/StringUtils.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "SUMOSAXAttributesImpl_Xerces.h"
#include "XMLSubSys.h"


// ===========================================================================
// class definitions
// ===========================================================================
GenericSAXHandler::GenericSAXHandler(
    StringBijection<int>::Entry* tags, int terminatorTag,
    StringBijection<int>::Entry* attrs, int terminatorAttr,
    const std::string& file, const std::string& expectedRoot)
    : myParentHandler(nullptr), myParentIndicator(SUMO_TAG_NOTHING), myFileName(file), myExpectedRoot(expectedRoot), mySchemaSeen(false) {
    int i = 0;
    while (tags[i].key != terminatorTag) {
        myTagMap.insert(TagMap::value_type(tags[i].str, tags[i].key));
        i++;
    }
    i = 0;
    while (attrs[i].key != terminatorAttr) {
        int key = attrs[i].key;
        assert(key >= 0);
        while (key >= (int)myPredefinedTags.size()) {
            myPredefinedTags.push_back(nullptr);
            myPredefinedTagsMML.push_back("");
        }
        myPredefinedTags[key] = convert(attrs[i].str);
        myPredefinedTagsMML[key] = attrs[i].str;
        i++;
    }
}


GenericSAXHandler::~GenericSAXHandler() {
    for (AttrMap::iterator i1 = myPredefinedTags.begin(); i1 != myPredefinedTags.end(); i1++) {
        delete[](*i1);
    }
}


void
GenericSAXHandler::setFileName(const std::string& name) {
    myFileName = name;
}


const std::string&
GenericSAXHandler::getFileName() const {
    return myFileName;
}


XMLCh*
GenericSAXHandler::convert(const std::string& name) const {
    int len = (int)name.length();
    XMLCh* ret = new XMLCh[len + 1];
    int i = 0;
    for (; i < len; i++) {
        ret[i] = (XMLCh) name[i];
    }
    ret[i] = 0;
    return ret;
}


void
GenericSAXHandler::startElement(const XMLCh* const /*uri*/,
                                const XMLCh* const /*localname*/,
                                const XMLCh* const qname,
                                const XERCES_CPP_NAMESPACE::Attributes& attrs) {
    std::string name = StringUtils::transcode(qname);
    if (mySchemaSeen && myExpectedRoot != "") {
        if (name != myExpectedRoot) {
            throw ProcessError("Found root element '" + name + "' in file '" + getFileName() + "' (expected '" + myExpectedRoot + "').");
        }
        mySchemaSeen = false;
    }
    int element = convertTag(name);
    myCharactersVector.clear();
    SUMOSAXAttributesImpl_Xerces na(attrs, myPredefinedTags, myPredefinedTagsMML, name);
    if (element == SUMO_TAG_INCLUDE) {
        std::string file = na.getString(SUMO_ATTR_HREF);
        if (!FileHelpers::isAbsolute(file)) {
            file = FileHelpers::getConfigurationRelative(getFileName(), file);
        }
        XMLSubSys::runParser(*this, file);
    } else {
        myStartElement(element, na);
    }
}


void
GenericSAXHandler::endElement(const XMLCh* const /*uri*/,
                              const XMLCh* const /*localname*/,
                              const XMLCh* const qname) {
    std::string name = StringUtils::transcode(qname);
    int element = convertTag(name);
    // collect characters
    if (myCharactersVector.size() != 0) {
        int len = 0;
        for (int i = 0; i < (int)myCharactersVector.size(); ++i) {
            len += (int)myCharactersVector[i].length();
        }
        char* buf = new char[len + 1];
        int pos = 0;
        for (int i = 0; i < (int)myCharactersVector.size(); ++i) {
            memcpy((unsigned char*) buf + pos, (unsigned char*) myCharactersVector[i].c_str(),
                   sizeof(char)*myCharactersVector[i].length());
            pos += (int)myCharactersVector[i].length();
        }
        buf[pos] = 0;

        // call user handler
        try {
            myCharacters(element, buf);
        } catch (std::runtime_error&) {
            delete[] buf;
            throw;
        }
        delete[] buf;
    }
    if (element != SUMO_TAG_INCLUDE) {
        myEndElement(element);
        if (myParentHandler && myParentIndicator == element) {
            XMLSubSys::setHandler(*myParentHandler);
            myParentIndicator = SUMO_TAG_NOTHING;
            myParentHandler = nullptr;
        }
    }
}


void
GenericSAXHandler::registerParent(const int tag, GenericSAXHandler* handler) {
    myParentHandler = handler;
    myParentIndicator = tag;
    XMLSubSys::setHandler(*this);
}


void
GenericSAXHandler::characters(const XMLCh* const chars,
                              const XERCES3_SIZE_t length) {
    myCharactersVector.push_back(StringUtils::transcode(chars, (int)length));
}


int
GenericSAXHandler::convertTag(const std::string& tag) const {
    TagMap::const_iterator i = myTagMap.find(tag);
    if (i == myTagMap.end()) {
        return SUMO_TAG_NOTHING;
    }
    return (*i).second;
}


std::string
GenericSAXHandler::buildErrorMessage(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    std::ostringstream buf;
    char* pMsg = XERCES_CPP_NAMESPACE::XMLString::transcode(exception.getMessage());
    buf << pMsg << std::endl;
    buf << " In file '" << getFileName() << "'" << std::endl;
    buf << " At line/column " << exception.getLineNumber() + 1
        << '/' << exception.getColumnNumber() << "." << std::endl;
    XERCES_CPP_NAMESPACE::XMLString::release(&pMsg);
    return buf.str();
}


void
GenericSAXHandler::warning(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    WRITE_WARNING(buildErrorMessage(exception));
}


void
GenericSAXHandler::error(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    throw ProcessError(buildErrorMessage(exception));
}


void
GenericSAXHandler::fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exception) {
    throw ProcessError(buildErrorMessage(exception));
}


void
GenericSAXHandler::myStartElement(int, const SUMOSAXAttributes&) {}


void
GenericSAXHandler::myCharacters(int, const std::string&) {}


void
GenericSAXHandler::myEndElement(int) {}


/****************************************************************************/
