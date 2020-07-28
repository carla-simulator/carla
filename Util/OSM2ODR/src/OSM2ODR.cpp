// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OSM2ODR.h"

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


namespace osm2odr {

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

  std::string ConvertOSMToOpenDRIVE(std::string osm_file, OSM2ODRSettings settings) {
    std::vector<std::string> OptionsArgs = {
      "--simple-projection",
      "--geometry.remove", "--ramps.guess", "--edges.join", "--junctions.join", "--roundabouts.guess",
      "--keep-edges.by-type",
      "highway.motorway,highway.motorway_link,highway.trunk,highway.trunk_link,highway.primary,highway.primary_link,highway.secondary,highway.secondary_link,highway.tertiary,highway.tertiary_link,highway.unclassified,highway.residential",
      "--tls.discard-loaded", "--tls.discard-simple", "--default.lanewidth",
      std::to_string(settings.default_lane_width),
      "--osm-files", "TRUE", "--opendrive-output", "TRUE", // necessary for now to enable osm input and xodr output
    };
    if (settings.elevation_layer_height > 0) {
      OptionsArgs.push_back("--osm.layer-elevation");
      OptionsArgs.push_back(std::to_string(settings.elevation_layer_height));
    }
    if (settings.use_offsets) {
      OptionsArgs.push_back("--offset.x");
      OptionsArgs.push_back(std::to_string(settings.offset_x));
      OptionsArgs.push_back("--offset.y");
      OptionsArgs.push_back(std::to_string(settings.offset_y));
    }

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

} // namespace OSM2ODR
