/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    OutputDevice.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    2004
///
// Static storage of an output device and its base (abstract) implementation
/****************************************************************************/
#include <config.h>

#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include "OutputDevice.h"
#include "OutputDevice_File.h"
#include "OutputDevice_COUT.h"
#include "OutputDevice_CERR.h"
#include "OutputDevice_Network.h"
#include "PlainXMLFormatter.h"
#include <utils/common/StringUtils.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>


// ===========================================================================
// static member definitions
// ===========================================================================
std::map<std::string, OutputDevice*> OutputDevice::myOutputDevices;


// ===========================================================================
// static method definitions
// ===========================================================================
OutputDevice&
OutputDevice::getDevice(const std::string& name) {
    // check whether the device has already been aqcuired
    if (myOutputDevices.find(name) != myOutputDevices.end()) {
        return *myOutputDevices[name];
    }
    // build the device
    OutputDevice* dev = nullptr;
    // check whether the device shall print to stdout
    if (name == "stdout") {
        dev = OutputDevice_COUT::getDevice();
    } else if (name == "stderr") {
        dev = OutputDevice_CERR::getDevice();
    } else if (FileHelpers::isSocket(name)) {
        try {
            int port = StringUtils::toInt(name.substr(name.find(":") + 1));
            dev = new OutputDevice_Network(name.substr(0, name.find(":")), port);
        } catch (NumberFormatException&) {
            throw IOError("Given port number '" + name.substr(name.find(":") + 1) + "' is not numeric.");
        } catch (EmptyData&) {
            throw IOError("No port number given.");
        }
    } else {
        const int len = (int)name.length();
        std::string name2 = name;
        if (OptionsCont::getOptions().isSet("output-prefix") && name != "/dev/null") {
            std::string prefix = OptionsCont::getOptions().getString("output-prefix");
            const std::string::size_type metaTimeIndex = prefix.find("TIME");
            if (metaTimeIndex != std::string::npos) {
                time_t rawtime;
                char buffer [80];
                time(&rawtime);
                struct tm* timeinfo = localtime(&rawtime);
                strftime(buffer, 80, "%Y-%m-%d-%H-%M-%S", timeinfo);
                prefix.replace(metaTimeIndex, 4, std::string(buffer));
            }
            name2 = FileHelpers::prependToLastPathComponent(prefix, name);
        }
        dev = new OutputDevice_File(name2, len > 3 && name.substr(len - 3) == ".gz");
    }
    dev->setPrecision();
    dev->getOStream() << std::setiosflags(std::ios::fixed);
    myOutputDevices[name] = dev;
    return *dev;
}


bool
OutputDevice::createDeviceByOption(const std::string& optionName,
                                   const std::string& rootElement,
                                   const std::string& schemaFile) {
    if (!OptionsCont::getOptions().isSet(optionName)) {
        return false;
    }
    OutputDevice& dev = OutputDevice::getDevice(OptionsCont::getOptions().getString(optionName));
    if (rootElement != "") {
        dev.writeXMLHeader(rootElement, schemaFile);
    }
    return true;
}


OutputDevice&
OutputDevice::getDeviceByOption(const std::string& optionName) {
    std::string devName = OptionsCont::getOptions().getString(optionName);
    if (myOutputDevices.find(devName) == myOutputDevices.end()) {
        throw InvalidArgument("Device '" + devName + "' has not been created.");
    }
    return OutputDevice::getDevice(devName);
}


void
OutputDevice::closeAll(bool keepErrorRetrievers) {
    std::vector<OutputDevice*> errorDevices;
    std::vector<OutputDevice*> nonErrorDevices;
    for (std::map<std::string, OutputDevice*>::iterator i = myOutputDevices.begin(); i != myOutputDevices.end(); ++i) {
        if (MsgHandler::getErrorInstance()->isRetriever(i->second)) {
            errorDevices.push_back(i->second);
        } else {
            nonErrorDevices.push_back(i->second);
        }
    }
    for (std::vector<OutputDevice*>::iterator i = nonErrorDevices.begin(); i != nonErrorDevices.end(); ++i) {
        try {
            //std::cout << "  close '" << (*i)->getFilename() << "'\n";
            (*i)->close();
        } catch (const IOError& e) {
            WRITE_ERROR("Error on closing output devices.");
            WRITE_ERROR(e.what());
        }
    }
    if (!keepErrorRetrievers) {
        for (std::vector<OutputDevice*>::iterator i = errorDevices.begin(); i != errorDevices.end(); ++i) {
            try {
                (*i)->close();
            } catch (const IOError& e) {
                std::cerr << "Error on closing error output devices." << std::endl;
                std::cerr << e.what() << std::endl;
            }
        }
    }
}


std::string
OutputDevice::realString(const double v, const int precision) {
    std::ostringstream oss;
    if (v == 0) {
        return "0";
    }
    if (v < pow(10., -precision)) {
        oss.setf(std::ios::scientific, std::ios::floatfield);
    } else {
        oss.setf(std::ios::fixed, std::ios::floatfield);     // use decimal format
        oss.setf(std::ios::showpoint);    // print decimal point
        oss << std::setprecision(precision);
    }
    oss << v;
    return oss.str();
}


// ===========================================================================
// member method definitions
// ===========================================================================
OutputDevice::OutputDevice(const int defaultIndentation, const std::string& filename) :
    myFilename(filename) {
    myFormatter = new PlainXMLFormatter(defaultIndentation);
}


OutputDevice::~OutputDevice() {
    delete myFormatter;
}


bool
OutputDevice::ok() {
    return getOStream().good();
}


const std::string&
OutputDevice::getFilename() {
    return myFilename;
}

void
OutputDevice::close() {
    while (closeTag()) {}
    for (std::map<std::string, OutputDevice*>::iterator i = myOutputDevices.begin(); i != myOutputDevices.end(); ++i) {
        if (i->second == this) {
            myOutputDevices.erase(i);
            break;
        }
    }
    MsgHandler::removeRetrieverFromAllInstances(this);
    delete this;
}


void
OutputDevice::setPrecision(int precision) {
    getOStream() << std::setprecision(precision);
}


bool
OutputDevice::writeXMLHeader(const std::string& rootElement,
                             const std::string& schemaFile,
                             std::map<SumoXMLAttr, std::string> attrs) {
    if (schemaFile != "") {
        attrs[SUMO_ATTR_XMLNS] = "http://www.w3.org/2001/XMLSchema-instance";
        attrs[SUMO_ATTR_SCHEMA_LOCATION] = "http://sumo.dlr.de/xsd/" + schemaFile;
    }
    return myFormatter->writeXMLHeader(getOStream(), rootElement, attrs);
}


OutputDevice&
OutputDevice::openTag(const std::string& xmlElement) {
    myFormatter->openTag(getOStream(), xmlElement);
    return *this;
}


OutputDevice&
OutputDevice::openTag(const SumoXMLTag& xmlElement) {
    myFormatter->openTag(getOStream(), xmlElement);
    return *this;
}


bool
OutputDevice::closeTag(const std::string& comment) {
    if (myFormatter->closeTag(getOStream(), comment)) {
        postWriteHook();
        return true;
    }
    return false;
}


void
OutputDevice::postWriteHook() {}


void
OutputDevice::inform(const std::string& msg, const char progress) {
    if (progress != 0) {
        getOStream() << msg << progress;
    } else {
        getOStream() << msg << '\n';
    }
    postWriteHook();
}


/****************************************************************************/
