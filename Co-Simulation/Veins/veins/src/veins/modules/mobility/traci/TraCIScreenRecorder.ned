//
// Copyright (C) 2006-2014 Christoph Sommer <sommer@ccs-labs.org>
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
// Simple support module to take (a series of) screenshots of a simulation running in the TraCI server.
//
// Note that the TraCI server needs to be run in GUI mode and support taking screenshots for this to work.
//
// The screenshots can then be converted to a video using something along the lines of
// mencoder 'mf://results/screenshot-*.png' -mf w=800:h=600:fps=25:type=png -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o output.avi
//
// See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
//
// @author Christoph Sommer
//
// @see TraCIScenarioManager
//
simple TraCIScreenRecorder
{
    parameters:
        @display("i=block/downarrow");
        @class(veins::TraCIScreenRecorder);
        string dirname = default("");  // directory name for screenshots (leave blank to use ${resultdir})
        string filenameTemplate = default("");  // file name for screenshots in printf syntax; the %f parameter is set to the current simulation time (leave blank to use screenshot-${runid}-@%08.2f.png)
        string viewName = default("View #0");  // which TraCI server view to choose
        double start @unit("s") = default(0s);  // when to take the first screenshot
        double interval @unit("s") = default(.1s);  // how often to take a screenshot
        double stop @unit("s") = default(-1s);  // when to take the last screenshot (-1: never)
}

