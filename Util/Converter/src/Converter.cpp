// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Converter.h"

#include <iostream>
#include <string>
#include <netimport/NIFrame.h>
#include <netimport/NILoader.h>
#include <netbuild/NBFrame.h>
#include <netbuild/NBNetBuilder.h>
#include <netwrite/NWFrame.h>
#include <utils/options/OptionsIO.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/MsgHandler.h>
#include <utils/distribution/DistributionCont.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>


namespace converter {

  void fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-c <CONFIGURATION>", "generate net with options read from file");
    oc.addCallExample("-n ./nodes.xml -e ./edges.xml -v -t ./owntypes.xml",
                      "generate net with given nodes, edges, and edge types doing verbose output");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // this subtopic is filled here, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    GeoConvHelper::addProjectionOptions(oc);
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Building Defaults");
    oc.addOptionSubTopic("TLS Building");
    oc.addOptionSubTopic("Ramp Guessing");
    oc.addOptionSubTopic("Edge Removal");
    oc.addOptionSubTopic("Unregulated Nodes");
    oc.addOptionSubTopic("Junctions");
    oc.addOptionSubTopic("Pedestrian");
    oc.addOptionSubTopic("Bicycle");
    oc.addOptionSubTopic("Railway");
    oc.addOptionSubTopic("Formats");
    SystemFrame::addReportOptions(oc); // this subtopic is filled here, too

    NIFrame::fillOptions();
    NBFrame::fillOptions(false);
    NWFrame::fillOptions(false);
    RandHelper::insertRandOptions();
}


  bool checkOptions() {
    bool ok = NIFrame::checkOptions();
    ok &= NBFrame::checkOptions();
    ok &= NWFrame::checkOptions();
    ok &= SystemFrame::checkOptions();
    return ok;
  }

  std::string ConvertOSMToOpenDRIVE(std::string osm_file, double offsetX, double offsetY) {
    // std::string OptionsArgs = "--geometry.remove --ramps.guess --edges.join --junctions.join --keep-edges.by-type highway.motorway,highway.motorway_link,highway.trunk,highway.trunk_link,highway.primary,highway.primary_link,highway.secondary,highway.secondary_link,highway.tertiary,highway.tertiary_link,highway.unclassified,highway.residential --tls.discard-loaded --tls.discard-simple --default.lanewidth 4.0 --osm.layer-elevation 4";
    std::vector<std::string> OptionsArgs = {
      "--proj", "+proj=tmerc",
      "--geometry.remove", "--ramps.guess", "--edges.join", "--junctions.join",
      "--keep-edges.by-type", "highway.motorway,highway.motorway_link,highway.trunk,highway.trunk_link,highway.primary,highway.primary_link,highway.secondary,highway.secondary_link,highway.tertiary,highway.tertiary_link,highway.unclassified,highway.residential",
      "--tls.discard-loaded", "--tls.discard-simple", "--default.lanewidth", "4.0",
      "--osm.layer-elevation", "4",
      "--osm-files", "TRUE", "--opendrive-output", "TRUE", // necessary for now to enable osm input and xodr output
      "--offset.x", std::to_string(offsetX), "--offset.y", std::to_string(offsetY)
    };

    // OptionsCont::getOptions().clear();
    OptionsCont& oc = OptionsCont::getOptions();
    oc.input_osm_file = osm_file;

    XMLSubSys::init();
    fillOptions();
    OptionsIO::setArgs(OptionsArgs);
    OptionsIO::getOptions();
    if (oc.processMetaOptions(OptionsArgs.size() < 2)) {
        SystemFrame::close();
        return 0;
    }
    XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"));
    if (oc.isDefault("aggregate-warnings")) {
        oc.set("aggregate-warnings", "5");
    }
    MsgHandler::initOutputOptions();
    if (!checkOptions()) {
        throw ProcessError();
    }
    RandHelper::initRandGlobal();
    // build the projection
    if (!GeoConvHelper::init(oc)) {
        throw ProcessError("Could not build projection!");
    }
    NBNetBuilder nb;
    nb.applyOptions(oc);
    // load data
    NILoader nl(nb);
    nl.load(oc);
    // flush aggregated errors and optionally ignore them
    MsgHandler::getErrorInstance()->clear(oc.getBool("ignore-errors"));
    // check whether any errors occurred
    if (MsgHandler::getErrorInstance()->wasInformed()) {
        throw ProcessError();
    }
    nb.compute(oc);
    // check whether any errors occurred
    if (MsgHandler::getErrorInstance()->wasInformed()) {
        throw ProcessError();
    }
    NWFrame::writeNetwork(oc, nb);


    DistributionCont::clear();
    SystemFrame::close();

    return oc.output_xodr_file;

  }

} // namespace converter
