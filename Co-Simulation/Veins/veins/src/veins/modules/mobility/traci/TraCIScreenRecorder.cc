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

#include "veins/modules/mobility/traci/TraCIScreenRecorder.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#ifdef _WIN32
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif /* _WIN32 */

using veins::TraCIScreenRecorder;

Define_Module(veins::TraCIScreenRecorder);

void TraCIScreenRecorder::initialize(int stage)
{
    if (stage == 0) {
        takeScreenshot = new cMessage("take screenshot");
        takeScreenshot->setSchedulingPriority(1); // this schedules screenshots after TraCI timesteps
        scheduleAt(par("start"), takeScreenshot);
        return;
    }
}

void TraCIScreenRecorder::handleMessage(cMessage* msg)
{
    ASSERT(msg == takeScreenshot);

    // get dirname
    const char* dirname = par("dirname").stringValue();
    if (std::string(dirname) == "") {
        dirname = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RESULTDIR);
    }

    // get absolute path of dirname (the TraCI server might be running in a different directory than OMNeT++)
    std::string dirname_abs;
    {
        char* s = realpath(dirname, nullptr);
        if (!s) {
            perror("cannot open output directory");
            throw cRuntimeError("cannot open output directory '%s'", dirname);
        }
        dirname_abs = s;
        free(s);
    }

    // get filename template
    std::string filenameTemplate = par("filenameTemplate").stdstringValue();
    if (filenameTemplate == "") {
        const char* myRunID = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID);
        filenameTemplate = "screenshot-" + std::string(myRunID) + "-@%08.2f.png";
#ifdef _WIN32
        // replace ':' with '-'
        for (std::string::iterator i = filenameTemplate.begin(); i != filenameTemplate.end(); ++i)
            if (*i == ':') *i = '-';
#endif /* _WIN32 */
    }

    // assemble filename
    std::string filename;
    {
        std::string tmpl = dirname_abs + "/" + filenameTemplate;
        char buf[1024];
        snprintf(buf, 1024, tmpl.c_str(), simTime().dbl());
        buf[1023] = 0;
        filename = buf;
    }

    // take screenshot
    TraCIScenarioManager* manager = TraCIScenarioManagerAccess().get();
    ASSERT(manager);
    TraCICommandInterface* traci = manager->getCommandInterface();
    if (!traci) {
        throw cRuntimeError("Cannot create screenshot: TraCI is not connected yet");
    }
    TraCICommandInterface::GuiView view = traci->guiView(par("viewName"));
    view.takeScreenshot(filename);

    // schedule next screenshot
    simtime_t stop = par("stop");
    if ((stop == -1) || (simTime() < stop)) {
        scheduleAt(simTime() + par("interval"), takeScreenshot);
    }
}

void TraCIScreenRecorder::finish()
{
    cancelAndDelete(takeScreenshot);
}
