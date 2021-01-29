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
/// @file    MSCFModel_IDMTest.cpp
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    2013-06-05
///
// Tests the cfmodel functions
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <gtest/gtest.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/options/OptionsCont.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSFrame.h>
#include <microsim/MSVehicleType.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSEdge.h>
#include <microsim/MSRoute.h>
#include <microsim/cfmodels/MSCFModel_IDM.h>


class MSVehicleMock : public MSVehicle {
public:
    MSVehicleMock(SUMOVehicleParameter* pars, const MSRoute* route,
                  MSVehicleType* type, const double speedFactor):
        MSVehicle(pars, route, type, speedFactor) {}

};


class MSCFModel_IDMTest : public testing::Test {
protected :
    MSVehicleType* type;
    SUMOVehicleParameter* defs;
    MSVehicle* veh;
    MSRoute* route;
    MSEdge* edge;
    MSLane* lane;
    double accel;
    double decel;
    double dawdle;
    double tau; // headway time

    virtual void SetUp() {
        if (!OptionsCont::getOptions().exists("step-length")) {
            MSFrame::fillOptions();
        }
        MSLane::initRNGs(OptionsCont::getOptions());
        tau = 1;
        MSGlobals::gUnitTests = true;
        defs = new SUMOVehicleParameter();
        defs->departLaneProcedure = DepartLaneDefinition::GIVEN;
        SUMOVTypeParameter typeDefs("t0");
        typeDefs.cfModel = SUMO_TAG_CF_IDM;
        //typeDefs.cfParameter[SUMO_ATTR_CF_IDM_STEPPING] = "1";
        ConstMSEdgeVector edges;
        MSEdge* dummyEdge = new MSEdge("dummy", 0, SumoXMLEdgeFunc::NORMAL, "", "", -1, 0);
        MSLane* dummyLane = new MSLane("dummy_0", 50 / 3.6, 100, dummyEdge, 0, PositionVector(), SUMO_const_laneWidth, SVCAll, 0, false, "");
        std::vector<MSLane*> lanes;
        lanes.push_back(dummyLane);
        dummyEdge->initialize(&lanes);
        edges.push_back(dummyEdge);
        route = new MSRoute("dummyRoute", edges, true, 0, defs->stops);
        MSGlobals::gActionStepLength = DELTA_T;
        type = MSVehicleType::build(typeDefs);
        veh = new MSVehicleMock(defs, route, type, 1);
        veh->setTentativeLaneAndPosition(dummyLane, 0);
        veh->initDevices();
        MSGlobals::gSemiImplicitEulerUpdate = true;
    }

    virtual void TearDown() {
        delete veh;
        delete type;
        delete route;
    }
};

/* Test the method 'brakeGap'.*/

TEST_F(MSCFModel_IDMTest, test_method_brakeGap) {
    // discrete braking model. keep driving for 1 s
    MSCFModel& m = type->getCarFollowModel();
    const double v = 3;
    EXPECT_DOUBLE_EQ(tau * v, m.brakeGap(v));
}

TEST_F(MSCFModel_IDMTest, test_method_getSecureGap) {
    // the value of getSecureGap should be consistent with followSpeed so that
    // strong braking is avoided after lane changing (#4517)
    MSCFModel& m = type->getCarFollowModel();
    for (double v = 0; v < 15; v += 1) { // follower
        for (double u = 0; u < 25; u += 1) { // leader
            double sg = m.getSecureGap(veh, nullptr, v, u, m.getMaxDecel());
            double vFollow = m.followSpeed(veh, v, sg, u, m.getMaxDecel(), nullptr);
            //std::cout << v << " " << u << " " << sg << " " << vFollow << " " << SPEED2ACCEL(vFollow - v) << "\n";
            EXPECT_GT(SPEED2ACCEL(vFollow - v), -2.2);
        }
    }
}



