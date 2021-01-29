# SUMOActivityGen

Activity-based Mobility Generation for SUMO Simulator

Contacts: Lara CODECA [lara.codeca@gmail.com]

Cite: L. Codeca and J. Haerri, "Multi-modal Activity-based Mobility Generation Applied to Microscopic Traffic Simulations", MobilTUM 2019

This program and the accompanying materials are made available under the
terms of the Eclipse Public License 2.0 which is available at <http://www.eclipse.org/legal/epl-2.0>.

This Source Code may also be made available under the following Secondary Licenses when the conditions for such availability set forth in the Eclipse Public License, v. 2.0 are satisfied: GNU General Public License version 3 <https://www.gnu.org/licenses/gpl-3.0.txt>.

## Overview

This project is in its early stages, and it is still under active development.

* The complete generation of a scenario from OSM can be done using `python3 scenarioFromOSM.py --osm {osm file} --out {output directory}`. All the generated files are going to be in the output directory.
* Alternatively, it can be tested and explored using the configuration files provided by [MoSTScenario](https://github.com/lcodeca/MoSTScenario) and in the `example` directory, starting from `bash most.generator.sh`.

The documentation is availalbe in the `docs` folder.

### Due to some changes in the SUMO development version of the TraCI APIs, the master branch may not compatible with SUMO 1.4.0

* _Release v0.2 is compatible with SUMO 1.4.0_
* _Release v0.1 is compatible with SUMO 1.2.0_
