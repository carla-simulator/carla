//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.veins.modules.mobility.traci;

//
// Uses the TraCIScenarioManager to programmatically insert new vehicles at the TraCI server.
//
// This is done whenever the total number of active vehicles drops below a given number.
//
// See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
//
// @author Christoph Sommer, David Eckhoff, Falko Dressler, Zheng Yao, Tobias Mayer, Alvaro Torres Cortes, Luca Bedogni
//
// @see TraCIScenarioManager
//

simple TraCIVehicleInserter
{
    parameters:
        @display("i=block/source");
        @class(veins::TraCIVehicleInserter);

        int numVehicles = default(0);
        bool useRouteDistributions = default(false);
        int vehicleRngIndex = default(0); // index of the RNG stream to be used, all random numbers concerning the managed vehicles
}

