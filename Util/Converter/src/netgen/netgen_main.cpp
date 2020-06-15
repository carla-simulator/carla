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
/// @file    netgen_main.cpp
/// @author  Markus Hartinger
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mar, 2003
///
// Main for NETGENERATE
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <netgen/NGNet.h>
#include <netgen/NGRandomNetBuilder.h>
#include <netgen/NGFrame.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBFrame.h>
#include <netwrite/NWFrame.h>
#include <netimport/NITypeLoader.h>
#include <netimport/NIXMLTypesHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/RandHelper.h>
#include <utils/common/ToString.h>
#include <utils/distribution/RandomDistributor.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
void
fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-c <CONFIGURATION>", "create net from given configuration");
    oc.addCallExample("--grid [grid-network options] -o <OUTPUTFILE>", "create grid net");
    oc.addCallExample("--spider [spider-network options] -o <OUTPUTFILE>", "create spider net");
    oc.addCallExample("--rand [random-network options] -o <OUTPUTFILE>", "create random net");

    oc.setAdditionalHelpMessage(" Either \"--grid\", \"--spider\" or \"--rand\" must be supplied.\n  In dependance to these switches other options are used.");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // this subtopic is filled here, too
    oc.addOptionSubTopic("Grid Network");
    oc.addOptionSubTopic("Spider Network");
    oc.addOptionSubTopic("Random Network");
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Building Defaults");
    oc.addOptionSubTopic("TLS Building");
    oc.addOptionSubTopic("Edge Removal");
    oc.addOptionSubTopic("Unregulated Nodes");
    oc.addOptionSubTopic("Junctions");
    oc.addOptionSubTopic("Pedestrian");
    oc.addOptionSubTopic("Bicycle");
    SystemFrame::addReportOptions(oc); // this subtopic is filled here, too

    NGFrame::fillOptions();
    NBFrame::fillOptions(true);
    NWFrame::fillOptions(true);
    oc.doRegister("default-junction-type", 'j', new Option_String());
    oc.addSynonyme("default-junction-type", "junctions");
    oc.addDescription("default-junction-type", "Building Defaults", "[traffic_light|priority|right_before_left|traffic_light_right_on_red|priority_stop|allway_stop|...] Determines junction type (see wiki/Networks/PlainXML#Node_types)");
    RandHelper::insertRandOptions();
}


bool
checkOptions() {
    bool ok = NGFrame::checkOptions();
    ok &= NBFrame::checkOptions();
    ok &= NWFrame::checkOptions();
    ok &= SystemFrame::checkOptions();
    return ok;
}


NGNet*
buildNetwork(NBNetBuilder& nb) {
    OptionsCont& oc = OptionsCont::getOptions();

    // spider-net
    if (oc.getBool("spider")) {
        // check values
        bool hadError = false;
        if (oc.getInt("spider.arm-number") < 3) {
            WRITE_ERROR("Spider networks need at least 3 arms.");
            hadError = true;
        }
        if (oc.getInt("spider.circle-number") < 1) {
            WRITE_ERROR("Spider networks need at least one circle.");
            hadError = true;
        }
        if (oc.getFloat("spider.space-radius") < 10) {
            WRITE_ERROR("The radius of spider networks must be at least 10m.");
            hadError = true;
        }
        if (hadError) {
            throw ProcessError();
        }
        // build if everything's ok
        NGNet* net = new NGNet(nb);
        net->createSpiderWeb(oc.getInt("spider.arm-number"), oc.getInt("spider.circle-number"),
                             oc.getFloat("spider.space-radius"), !oc.getBool("spider.omit-center"));
        return net;
    }
    // grid-net
    if (oc.getBool("grid")) {
        // get options
        int xNo = oc.getInt("grid.x-number");
        int yNo = oc.getInt("grid.y-number");
        double xLength = oc.getFloat("grid.x-length");
        double yLength = oc.getFloat("grid.y-length");
        double attachLength = oc.getFloat("grid.attach-length");
        if (oc.isDefault("grid.x-number") && !oc.isDefault("grid.number")) {
            xNo = oc.getInt("grid.number");
        }
        if (oc.isDefault("grid.y-number") && !oc.isDefault("grid.number")) {
            yNo = oc.getInt("grid.number");
        }
        if (oc.isDefault("grid.x-length") && !oc.isDefault("grid.length")) {
            xLength = oc.getFloat("grid.length");
        }
        if (oc.isDefault("grid.y-length") && !oc.isDefault("grid.length")) {
            yLength = oc.getFloat("grid.length");
        }
        // check values
        bool hadError = false;
        if (attachLength == 0 && (xNo < 2 || yNo < 2)) {
            WRITE_ERROR("The number of nodes must be at least 2 in both directions.");
            hadError = true;
        }
        if (xLength < 10. || yLength < 10.) {
            WRITE_ERROR("The distance between nodes must be at least 10m in both directions.");
            hadError = true;
        }
        if (attachLength != 0.0 && attachLength < 10.) {
            WRITE_ERROR("The length of attached streets must be at least 10m.");
            hadError = true;
        }
        if (hadError) {
            throw ProcessError();
        }
        // build if everything's ok
        NGNet* net = new NGNet(nb);
        net->createChequerBoard(xNo, yNo, xLength, yLength, attachLength);
        return net;
    }
    // random net
    RandomDistributor<int> neighborDist;
    neighborDist.add(1, oc.getFloat("rand.neighbor-dist1"));
    neighborDist.add(2, oc.getFloat("rand.neighbor-dist2"));
    neighborDist.add(3, oc.getFloat("rand.neighbor-dist3"));
    neighborDist.add(4, oc.getFloat("rand.neighbor-dist4"));
    neighborDist.add(5, oc.getFloat("rand.neighbor-dist5"));
    neighborDist.add(6, oc.getFloat("rand.neighbor-dist6"));
    NGNet* net = new NGNet(nb);
    NGRandomNetBuilder randomNet(*net,
                                 DEG2RAD(oc.getFloat("rand.min-angle")),
                                 oc.getFloat("rand.min-distance"),
                                 oc.getFloat("rand.max-distance"),
                                 oc.getFloat("rand.connectivity"),
                                 oc.getInt("rand.num-tries"),
                                 neighborDist);
    randomNet.createNet(oc.getInt("rand.iterations"), oc.getBool("rand.grid"));
    return net;
}



int
main(int argc, char** argv) {
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("Synthetic network generator for the microscopic, multi-modal traffic simulation SUMO.");
    oc.setApplicationName("netgenerate", "Eclipse SUMO netgenerate Version " VERSION_STRING);
    int ret = 0;
    try {
        // initialise the application system (messaging, xml, options)
        XMLSubSys::init();
        fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions();
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"));
        MsgHandler::initOutputOptions();
        if (!checkOptions()) {
            throw ProcessError();
        }
        GeoConvHelper::init("!",
                            Position(oc.getFloat("offset.x"), oc.getFloat("offset.y")),
                            Boundary(), Boundary());
        RandHelper::initRandGlobal();
        NBNetBuilder nb;
        nb.applyOptions(oc);
        if (oc.isSet("type-files")) {
            NIXMLTypesHandler* handler = new NIXMLTypesHandler(nb.getTypeCont());
            NITypeLoader::load(handler, oc.getStringVector("type-files"), "types");
        }
        // build the netgen-network description
        NGNet* net = buildNetwork(nb);
        // ... and we have to do this...
        oc.resetWritable();
        // transfer to the netbuilding structures
        net->toNB();
        delete net;
        // report generated structures
        WRITE_MESSAGE(" Generation done;");
        WRITE_MESSAGE("   " + toString<int>(nb.getNodeCont().size()) + " nodes generated.");
        WRITE_MESSAGE("   " + toString<int>(nb.getEdgeCont().size()) + " edges generated.");
        nb.compute(oc);
        NWFrame::writeNetwork(oc, nb);
    } catch (const ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
#ifndef _DEBUG
    } catch (const std::exception& e) {
        if (std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on unknown error).", false);
        ret = 1;
#endif
    }
    SystemFrame::close();
    if (ret == 0) {
        std::cout << "Success." << std::endl;
    }
    return ret;
}


/****************************************************************************/
