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
/// @file    SystemFrame.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 23.06.2003
///
// A set of actions common to all applications
/****************************************************************************/
#include <config.h>

#include "SystemFrame.h"
#include <string>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StdDefs.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include "RandHelper.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
SystemFrame::addConfigurationOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("Configuration");

    oc.doRegister("configuration-file", 'c', new Option_FileName());
    oc.addSynonyme("configuration-file", "configuration");
    oc.addDescription("configuration-file", "Configuration", "Loads the named config on startup");
    oc.addXMLDefault("configuration-file");

    oc.doRegister("save-configuration", 'C', new Option_FileName());
    oc.addSynonyme("save-config", "save-configuration");
    oc.addDescription("save-configuration", "Configuration", "Saves current configuration into FILE");

    oc.doRegister("save-template", new Option_FileName());
    oc.addDescription("save-template", "Configuration", "Saves a configuration template (empty) into FILE");

    oc.doRegister("save-schema", new Option_FileName());
    oc.addDescription("save-schema", "Configuration", "Saves the configuration schema into FILE");

    oc.doRegister("save-commented", new Option_Bool(false));
    oc.addSynonyme("save-commented", "save-template.commented");
    oc.addDescription("save-commented", "Configuration", "Adds comments to saved template, configuration, or schema");
}


void
SystemFrame::addReportOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("Report");

    oc.doRegister("verbose", 'v', new Option_Bool(false));
    oc.addDescription("verbose", "Report", "Switches to verbose output");

    oc.doRegister("print-options", new Option_Bool(false));
    oc.addDescription("print-options", "Report", "Prints option values before processing");

    oc.doRegister("help", '?', new Option_BoolExtended(false));
    oc.addDescription("help", "Report", "Prints this screen or selected topics");

    oc.doRegister("version", 'V', new Option_Bool(false));
    oc.addDescription("version", "Report", "Prints the current version");

    oc.doRegister("xml-validation", 'X', new Option_String("auto"));
    oc.addDescription("xml-validation", "Report", "Set schema validation scheme of XML inputs (\"never\", \"auto\" or \"always\")");

    oc.doRegister("xml-validation.net", new Option_String("never"));
    oc.addDescription("xml-validation.net", "Report", "Set schema validation scheme of SUMO network inputs (\"never\", \"auto\" or \"always\")");

    oc.doRegister("no-warnings", 'W', new Option_Bool(false));
    oc.addSynonyme("no-warnings", "suppress-warnings", true);
    oc.addDescription("no-warnings", "Report", "Disables output of warnings");

    oc.doRegister("aggregate-warnings", new Option_Integer(-1));
    oc.addDescription("aggregate-warnings", "Report", "Aggregate warnings of the same type whenever more than INT occur");

    oc.doRegister("log", 'l', new Option_FileName());
    oc.addSynonyme("log", "log-file");
    oc.addDescription("log", "Report", "Writes all messages to FILE (implies verbose)");

    oc.doRegister("message-log", new Option_FileName());
    oc.addDescription("message-log", "Report", "Writes all non-error messages to FILE (implies verbose)");

    oc.doRegister("error-log", new Option_FileName());
    oc.addDescription("error-log", "Report", "Writes all warnings and errors to FILE");

    oc.doRegister("write-license", new Option_Bool(false));
    oc.addDescription("write-license", "Output", "Include license info into every output file");

    oc.doRegister("output-prefix", new Option_String());
    oc.addDescription("output-prefix", "Output", "Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time.");

    oc.doRegister("precision", new Option_Integer(2));
    oc.addDescription("precision", "Output", "Defines the number of digits after the comma for floating point output");

    oc.doRegister("precision.geo", new Option_Integer(6));
    oc.addDescription("precision.geo", "Output", "Defines the number of digits after the comma for lon,lat output");

    oc.doRegister("human-readable-time", 'H', new Option_Bool(false));
    oc.addDescription("human-readable-time", "Output", "Write time values as hour:minute:second or day:hour:minute:second rathern than seconds");
}


bool
SystemFrame::checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    gPrecision = oc.getInt("precision");
    gPrecisionGeo = oc.getInt("precision.geo");
    gHumanReadableTime = oc.getBool("human-readable-time");
    if (oc.exists("weights.random-factor")) {
        gWeightsRandomFactor = oc.getFloat("weights.random-factor");
    }
    return true;
}


void
SystemFrame::close() {
    // flush aggregated warnings
    MsgHandler::getWarningInstance()->clear();
    // close all output devices
    OutputDevice::closeAll();
    // close the xml-subsystem
    XMLSubSys::close();
    // delete build program options
    OptionsCont::getOptions().clear();
    // delete messages
    MsgHandler::cleanupOnEnd();
}


/****************************************************************************/
