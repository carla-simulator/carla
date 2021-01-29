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
/// @file    MSCFModel_CC.cpp
/// @author  Michele Segata
/// @date    Wed, 18 Apr 2012
///
// A series of automatic Cruise Controllers (CC, ACC, CACC)
/****************************************************************************/
#include "MSCFModel_CC.h"
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/StringUtils.h>
#include <microsim/cfmodels/ParBuffer.h>
#include <libsumo/Vehicle.h>
#include <libsumo/TraCIDefs.h>
#include <algorithm>

#ifndef sgn
#define sgn(x) ((x > 0) - (x < 0))
#endif

// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_CC::MSCFModel_CC(const MSVehicleType* vtype) : MSCFModel(vtype),
    myCcDecel(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_CCDECEL, 1.5)),
    myCcAccel(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_CCACCEL, 1.5)),
    myConstantSpacing(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_CONSTSPACING, 5.0)),
    myKp(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_KP, 1.0)),
    myLambda(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_LAMBDA, 0.1)),
    myC1(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_C1, 0.5)),
    myXi(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_XI, 1.0)),
    myOmegaN(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_OMEGAN, 0.2)),
    myTau(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_TAU, 0.5)),
    myLanesCount((int)vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_LANES_COUNT, -1)),
    myPloegH(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_PLOEG_H, 0.5)),
    myPloegKp(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_PLOEG_KP, 0.2)),
    myPloegKd(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_PLOEG_KD, 0.7)),
    myFlatbedKa(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_FLATBED_KA, 2.4)),
    myFlatbedKv(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_FLATBED_KV, 0.6)),
    myFlatbedKp(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_FLATBED_KP, 12.0)),
    myFlatbedH(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_FLATBED_H, 4.0)),
    myFlatbedD(vtype->getParameter().getCFParam(SUMO_ATTR_CF_CC_FLATBED_D, 5.0)) {

    //if the lanes count has not been specified in the attributes of the model, lane changing cannot properly work
    if (myLanesCount == -1) {
        throw ProcessError("The number of lanes needs to be specified in the attributes of carFollowing-CC with the \"lanesCount\" attribute");
    }

    //instantiate the driver model. For now, use Krauss as default, then needs to be parameterized
    myHumanDriver = new MSCFModel_Krauss(vtype);

}

MSCFModel_CC::~MSCFModel_CC() {}

MSCFModel::VehicleVariables*
MSCFModel_CC::createVehicleVariables() const {
    CC_VehicleVariables* vars = new CC_VehicleVariables();
    vars->ccKp = myKp;
    vars->accLambda = myLambda;
    vars->caccSpacing = myConstantSpacing;
    vars->caccC1 = myC1;
    vars->caccXi = myXi;
    vars->caccOmegaN = myOmegaN;
    vars->engineTau = myTau;
    //we cannot invoke recomputeParameters() because we have no pointer to the MSVehicle class
    vars->caccAlpha1 = 1 - vars->caccC1;
    vars->caccAlpha2 = vars->caccC1;
    vars->caccAlpha3 = -(2 * vars->caccXi - vars->caccC1 * (vars->caccXi + sqrt(vars->caccXi * vars->caccXi - 1))) * vars->caccOmegaN;
    vars->caccAlpha4 = -(vars->caccXi + sqrt(vars->caccXi * vars->caccXi - 1)) * vars->caccOmegaN * vars->caccC1;
    vars->caccAlpha5 = -vars->caccOmegaN * vars->caccOmegaN;

    vars->ploegH = myPloegH;
    vars->ploegKp = myPloegKp;
    vars->ploegKd = myPloegKd;
    vars->flatbedKa = myFlatbedKa;
    vars->flatbedKv = myFlatbedKv;
    vars->flatbedKp = myFlatbedKp;
    vars->flatbedD = myFlatbedD;
    vars->flatbedH = myFlatbedH;
    //by default use a first order lag model for the engine
    vars->engine = new FirstOrderLagModel();
    vars->engine->setParameter(FOLM_PAR_TAU, vars->engineTau);
    vars->engine->setParameter(FOLM_PAR_DT, TS);
    vars->engine->setMaximumAcceleration(myAccel);
    vars->engine->setMaximumDeceleration(myDecel);
    vars->engineModel = CC_ENGINE_MODEL_FOLM;
    return (VehicleVariables*)vars;
}

void
MSCFModel_CC::performAutoLaneChange(MSVehicle* const veh) const {
    bool canChange;
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    // check for left lane change
    std::pair<int, int> state = libsumo::Vehicle::getLaneChangeState(veh->getID(), +1);
    int traciState = state.first;
    if (traciState & LCA_LEFT && traciState & LCA_SPEEDGAIN) {
        // we can gain by moving left. check that all vehicles can move left
        if (!(state.first & LCA_BLOCKED)) {
            // leader is not blocked. check all the members
            canChange = true;
            for (auto m = vars->members.begin(); m != vars->members.end(); m++) {
                const std::pair<int, int> mState = libsumo::Vehicle::getLaneChangeState(m->second, +1);
                if (mState.first & LCA_BLOCKED) {
                    canChange = false;
                    break;
                }
            }
            if (canChange) {
                libsumo::Vehicle::changeLane(veh->getID(), veh->getLaneIndex() + 1, 0);
                for (auto m = vars->members.begin(); m != vars->members.end(); m++) {
                    libsumo::Vehicle::changeLane(m->second, veh->getLaneIndex() + 1, 0);
                }
            }

        }
    }
    state = libsumo::Vehicle::getLaneChangeState(veh->getID(), -1);
    traciState = state.first;
    if (traciState & LCA_RIGHT && traciState & LCA_KEEPRIGHT) {
        // we should move back right. check that all vehicles can move right
        if (!(state.first & LCA_BLOCKED)) {
            // leader is not blocked. check all the members
            canChange = true;
            for (auto m = vars->members.begin(); m != vars->members.end(); m++) {
                const std::pair<int, int> mState = libsumo::Vehicle::getLaneChangeState(m->second, -1);
                if (mState.first & LCA_BLOCKED) {
                    canChange = false;
                    break;
                }
            }
            if (canChange) {
                libsumo::Vehicle::changeLane(veh->getID(), veh->getLaneIndex() - 1, 1);
                for (auto m = vars->members.begin(); m != vars->members.end(); m++) {
                    libsumo::Vehicle::changeLane(m->second, veh->getLaneIndex() - 1, 1);
                }
            }

        }
    }

}

double
MSCFModel_CC::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    double vNext;
    //acceleration computed by the controller
    double controllerAcceleration;
    //acceleration after engine actuation
    double engineAcceleration;

    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();

    //call processNextStop() to ensure vehicle removal in case of crash
    veh->processNextStop(vPos);

    //check whether the vehicle has collided and set the flag in case
    if (!vars->crashed) {
        for (const MSVehicle::Stop& s : veh->getStops()) {
            if (s.collision) {
                vars->crashed = true;
            }
        }
    }

    if (vars->activeController != Plexe::DRIVER) {
        veh->setChosenSpeedFactor(vars->ccDesiredSpeed / veh->getLane()->getSpeedLimit());
    }

    if (vars->autoLaneChange) {
        performAutoLaneChange(veh);
    }

    if (vars->activeController != Plexe::DRIVER) {
        controllerAcceleration = SPEED2ACCEL(vPos - veh->getSpeed());
        controllerAcceleration = std::min(vars->uMax, std::max(vars->uMin, controllerAcceleration));
        //compute the actual acceleration applied by the engine
        engineAcceleration = vars->engine->getRealAcceleration(veh->getSpeed(), veh->getAcceleration(), controllerAcceleration, MSNet::getInstance()->getCurrentTimeStep());
        vNext = MAX2(double(0), veh->getSpeed() + ACCEL2SPEED(engineAcceleration));
        vars->controllerAcceleration = controllerAcceleration;
    } else {
        vNext = myHumanDriver->finalizeSpeed(veh, vPos);
    }

    return vNext;
}


double
MSCFModel_CC::followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred) const {

    UNUSED_PARAMETER(pred);
    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();

    if (vars->activeController != Plexe::DRIVER) {
        return _v(veh, gap2pred, speed, predSpeed);
    } else {
        return myHumanDriver->followSpeed(veh, speed, gap2pred, predSpeed, predMaxDecel);
    }
}

double
MSCFModel_CC::insertionFollowSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const /*pred*/) const {
    UNUSED_PARAMETER(veh);
    UNUSED_PARAMETER(gap2pred);
    UNUSED_PARAMETER(predSpeed);
    UNUSED_PARAMETER(predMaxDecel);
    //by returning speed + 1, we tell sumo that "speed" is always a safe speed
    return speed + 1;
}

double
MSCFModel_CC::stopSpeed(const MSVehicle* const veh, double speed, double gap2pred) const {

    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    if (vars->activeController != Plexe::DRIVER) {
        double gap2pred, relSpeed;
        getRadarMeasurements(veh, gap2pred, relSpeed);
        if (gap2pred == -1) {
            gap2pred = std::numeric_limits<double>().max();
        }
        return _v(veh, gap2pred, speed, speed + relSpeed);
    } else {
        return myHumanDriver->stopSpeed(veh, speed, gap2pred);
    }
}

double MSCFModel_CC::freeSpeed(const MSVehicle* const veh, double speed, double seen, double maxSpeed, const bool onInsertion) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    if (vars->activeController != Plexe::DRIVER) {
        double gap2pred, relSpeed;
        getRadarMeasurements(veh, gap2pred, relSpeed);
        if (gap2pred == -1) {
            gap2pred = std::numeric_limits<double>().max();
        }
        return _v(veh, gap2pred, speed, speed + relSpeed);
    } else {
        return MSCFModel::freeSpeed(veh, speed, seen, maxSpeed, onInsertion);
    }
}

double
MSCFModel_CC::interactionGap(const MSVehicle* const veh, double vL) const {

    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    if (vars->activeController != Plexe::DRIVER) {
        //maximum radar range is CC is enabled
        return 250;
    } else {
        return myHumanDriver->interactionGap(veh, vL);
    }

}

double
MSCFModel_CC::maxNextSpeed(double speed, const MSVehicle* const veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    if (vars->engineModel == CC_ENGINE_MODEL_FOLM) {
        return speed + (double) ACCEL2SPEED(getMaxAccel());
    } else {
        return speed + (double) ACCEL2SPEED(20);
    }
}

double
MSCFModel_CC::minNextSpeed(double speed, const MSVehicle* const veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    if (vars->engineModel == CC_ENGINE_MODEL_FOLM) {
        return MSCFModel::minNextSpeed(speed, veh);
    } else {
        return MAX2((double)0, speed - (double) ACCEL2SPEED(20));
    }
}

double
MSCFModel_CC::_v(const MSVehicle* const veh, double gap2pred, double egoSpeed, double predSpeed) const {

    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();

    //acceleration computed by the controller
    double controllerAcceleration = vars->fixedAcceleration;
    //speed computed by the model
    double speed;
    //acceleration computed by the Cruise Control
    double ccAcceleration;
    //acceleration computed by the Adaptive Cruise Control
    double accAcceleration;
    //acceleration computed by the Cooperative Adaptive Cruise Control
    double caccAcceleration;
    //variables needed by CACC
    double predAcceleration, leaderAcceleration, leaderSpeed;
    //dummy variables used for auto feeding
    Position pos;
    double time;
    const double currentTime = STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() + DELTA_T);

    if (vars->crashed) {
        return 0;
    }
    if (vars->activeController == Plexe::DRIVER || !vars->useFixedAcceleration) {
        switch (vars->activeController) {
            case Plexe::ACC:
                ccAcceleration = _cc(veh, egoSpeed, vars->ccDesiredSpeed);
                accAcceleration = _acc(veh, egoSpeed, predSpeed, gap2pred, vars->accHeadwayTime);
                if (gap2pred > 250 || ccAcceleration < accAcceleration) {
                    controllerAcceleration = ccAcceleration;
                } else {
                    controllerAcceleration = accAcceleration;
                }
                break;

            case Plexe::CACC:
                if (vars->autoFeed) {
                    getVehicleInformation(vars->leaderVehicle, vars->leaderSpeed, vars->leaderAcceleration, vars->leaderControllerAcceleration, pos, time);
                    getVehicleInformation(vars->frontVehicle, vars->frontSpeed, vars->frontAcceleration, vars->frontControllerAcceleration, pos, time);
                }

                if (vars->useControllerAcceleration) {
                    predAcceleration = vars->frontControllerAcceleration;
                    leaderAcceleration = vars->leaderControllerAcceleration;
                } else {
                    predAcceleration = vars->frontAcceleration;
                    leaderAcceleration = vars->leaderAcceleration;
                }
                //overwrite pred speed using data obtained through wireless communication
                predSpeed = vars->frontSpeed;
                leaderSpeed = vars->leaderSpeed;
                if (vars->usePrediction) {
                    predSpeed += (currentTime - vars->frontDataReadTime) * vars->frontAcceleration;
                    leaderSpeed += (currentTime - vars->leaderDataReadTime) * vars->leaderAcceleration;
                }

                if (vars->caccInitialized) {
                    controllerAcceleration = _cacc(veh, egoSpeed, predSpeed, predAcceleration, gap2pred, leaderSpeed, leaderAcceleration, vars->caccSpacing);
                } else
                    //do not let CACC take decisions until at least one packet has been received
                {
                    controllerAcceleration = 0;
                }

                break;

            case Plexe::FAKED_CACC:

                if (vars->autoFeed) {
                    getVehicleInformation(vars->leaderVehicle, vars->fakeData.leaderSpeed, vars->fakeData.leaderAcceleration, vars->fakeData.leaderControllerAcceleration, pos, time);
                    getVehicleInformation(vars->frontVehicle, vars->fakeData.frontSpeed, vars->fakeData.frontAcceleration, vars->fakeData.frontControllerAcceleration, pos, time);
                    vars->fakeData.frontDistance = pos.distanceTo2D(veh->getPosition());
                }

                if (vars->useControllerAcceleration) {
                    predAcceleration = vars->fakeData.frontControllerAcceleration;
                    leaderAcceleration = vars->fakeData.leaderControllerAcceleration;
                } else {
                    predAcceleration = vars->fakeData.frontAcceleration;
                    leaderAcceleration = vars->fakeData.leaderAcceleration;
                }
                ccAcceleration = _cc(veh, egoSpeed, vars->ccDesiredSpeed);
                caccAcceleration = _cacc(veh, egoSpeed, vars->fakeData.frontSpeed, predAcceleration, vars->fakeData.frontDistance, vars->fakeData.leaderSpeed, leaderAcceleration, vars->caccSpacing);
                //faked CACC can be used to get closer to a platoon for joining
                //using the minimum acceleration ensures that we do not exceed
                //the CC desired speed
                controllerAcceleration = std::min(ccAcceleration, caccAcceleration);

                break;

            case Plexe::PLOEG:

                if (vars->autoFeed) {
                    getVehicleInformation(vars->frontVehicle, vars->frontSpeed, vars->frontAcceleration, vars->frontControllerAcceleration, pos, time);
                }

                if (vars->useControllerAcceleration) {
                    predAcceleration = vars->frontControllerAcceleration;
                } else {
                    predAcceleration = vars->frontAcceleration;
                }
                //check if we received at least one packet
                if (vars->frontInitialized)
                    //ploeg's controller computes \dot{u}_i, so we need to sum such value to the previously computed u_i
                {
                    controllerAcceleration = vars->controllerAcceleration + _ploeg(veh, egoSpeed, predSpeed, predAcceleration, gap2pred);
                } else {
                    controllerAcceleration = 0;
                }

                break;

            case Plexe::CONSENSUS:
                controllerAcceleration = _consensus(veh, egoSpeed, veh->getPosition(), currentTime);
                break;

            case Plexe::FLATBED:

                if (vars->autoFeed) {
                    getVehicleInformation(vars->leaderVehicle, vars->leaderSpeed, vars->leaderAcceleration, vars->leaderControllerAcceleration, pos, time);
                    getVehicleInformation(vars->frontVehicle, vars->frontSpeed, vars->frontAcceleration, vars->frontControllerAcceleration, pos, time);
                }

                //overwrite pred speed using data obtained through wireless communication
                predSpeed = vars->frontSpeed;
                leaderSpeed = vars->leaderSpeed;
                if (vars->usePrediction) {
                    predSpeed += (currentTime - vars->frontDataReadTime) * vars->frontAcceleration;
                    leaderSpeed += (currentTime - vars->leaderDataReadTime) * vars->leaderAcceleration;
                }

                if (vars->caccInitialized) {
                    controllerAcceleration = _flatbed(veh, veh->getAcceleration(), egoSpeed, predSpeed, gap2pred, leaderSpeed);
                } else
                    //do not let CACC take decisions until at least one packet has been received
                {
                    controllerAcceleration = 0;
                }

                break;

            case Plexe::DRIVER:
                std::cerr << "Switching to normal driver behavior still not implemented in MSCFModel_CC\n";
                assert(false);
                break;

            default:
                std::cerr << "Invalid controller selected in MSCFModel_CC\n";
                assert(false);
                break;

        }

    }

    speed = MAX2(double(0), egoSpeed + ACCEL2SPEED(controllerAcceleration));

    return speed;
}

double
MSCFModel_CC::_cc(const MSVehicle* veh, double egoSpeed, double desSpeed) const {

    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    //Eq. 5.5 of the Rajamani book, with Ki = 0 and bounds on max and min acceleration
    return std::min(myCcAccel, std::max(-myCcDecel, -vars->ccKp * (egoSpeed - desSpeed)));

}

double
MSCFModel_CC::_acc(const MSVehicle* veh, double egoSpeed, double predSpeed, double gap2pred, double headwayTime) const {

    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    //Eq. 6.18 of the Rajamani book
    return -1.0 / headwayTime * (egoSpeed - predSpeed + vars->accLambda * (-gap2pred + headwayTime * egoSpeed + 2));

}

double
MSCFModel_CC::_cacc(const MSVehicle* veh, double egoSpeed, double predSpeed, double predAcceleration, double gap2pred, double leaderSpeed, double leaderAcceleration, double spacing) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    //compute epsilon, i.e., the desired distance error
    double epsilon = -gap2pred + spacing; //NOTICE: error (if any) should already be included in gap2pred
    //compute epsilon_dot, i.e., the desired speed error
    double epsilon_dot = egoSpeed - predSpeed;
    //Eq. 7.39 of the Rajamani book
    return vars->caccAlpha1 * predAcceleration + vars->caccAlpha2 * leaderAcceleration +
           vars->caccAlpha3 * epsilon_dot + vars->caccAlpha4 * (egoSpeed - leaderSpeed) + vars->caccAlpha5 * epsilon;
}


double
MSCFModel_CC::_ploeg(const MSVehicle* veh, double egoSpeed, double predSpeed, double predAcceleration, double gap2pred) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    return (1 / vars->ploegH * (
                -vars->controllerAcceleration +
                vars->ploegKp * (gap2pred - (2 + vars->ploegH * egoSpeed)) +
                vars->ploegKd * (predSpeed - egoSpeed - vars->ploegH * veh->getAcceleration()) +
                predAcceleration
            )) * TS ;
}

double
MSCFModel_CC::d_i_j(const struct Plexe::VEHICLE_DATA* vehicles, const double h[MAX_N_CARS], int i, int j) const {

    int k, min_i, max_i;
    double d = 0;
    //compute indexes of the summation
    if (j < i) {
        min_i = j;
        max_i = i - 1;
    } else {
        min_i = i;
        max_i = j - 1;
    }
    //compute distance
    for (k = min_i; k <= max_i; k++) {
        d += h[k] * vehicles[0].speed + vehicles[k].length + 15;
    }

    if (j < i) {
        return d;
    } else {
        return -d;
    }

}

double
MSCFModel_CC::_consensus(const MSVehicle* veh, double egoSpeed, Position egoPosition, double time) const {
    //TODO: this controller, by using real GPS coordinates, does only work
    //when vehicles are traveling west-to-east on a straight line, basically
    //on the X axis. This needs to be fixed to consider direction as well
    CC_VehicleVariables* vars = (CC_VehicleVariables*)veh->getCarFollowVariables();
    int index = vars->position;
    int nCars = vars->nCars;
    struct Plexe::VEHICLE_DATA* vehicles = vars->vehicles;

    //loop variable
    int j;
    //control input
    double u_i = 0;
    //actual distance term
    double actualDistance = 0;
    //desired distance term
    double desiredDistance = 0;
    //speed error term
    double speedError = 0;
    //degree of agent i
    double d_i = 0;

    //compensate my position: compute prediction of what will be my position at time of actuation
    Position egoVelocity = veh->getVelocityVector();
    egoPosition.set(egoPosition.x() + egoVelocity.x() * STEPS2TIME(DELTA_T),
                    egoPosition.y() + egoVelocity.y() * STEPS2TIME(DELTA_T));
    vehicles[index].speed = egoSpeed;
    vehicles[index].positionX = egoPosition.x();
    vehicles[index].positionY = egoPosition.y();

    //check that data from all vehicles have been received. the control
    //law might actually need a subset of all the data, but d_i_j needs
    //the lengths of all vehicles. uninitialized values might cause problems
    if (vars->nInitialized != vars->nCars - 1) {
        return 0;
    }

    //compute speed error.
    speedError = -vars->b[index] * (egoSpeed - vehicles[0].speed);

    //compute desired distance term
    for (j = 0; j < nCars; j++) {
        if (j == index) {
            continue;
        }
        d_i += vars->L[index][j];
        desiredDistance -= vars->K[index][j] * vars->L[index][j] * d_i_j(vehicles, vars->h, index, j);
    }
    desiredDistance = desiredDistance / d_i;

    //compute actual distance term
    for (j = 0; j < nCars; j++) {
        if (j == index) {
            continue;
        }
        //distance error for consensus with GPS equipped
        Position otherPosition;
        double dt = time - vehicles[j].time;
        //predict the position of the other vehicle
        otherPosition.setx(vehicles[j].positionX + dt * vehicles[j].speedX);
        otherPosition.sety(vehicles[j].positionY + dt * vehicles[j].speedY);
        double distance = egoPosition.distanceTo2D(otherPosition) * sgn(j - index);
        actualDistance -= vars->K[index][j] * vars->L[index][j] * distance;
    }

    actualDistance = actualDistance / (d_i);

    //original paper formula
    u_i = (speedError + desiredDistance + actualDistance) / 1000;

    return u_i;
}

double
MSCFModel_CC::_flatbed(const MSVehicle* veh, double egoAcceleration, double egoSpeed, double predSpeed,
                       double gap2pred, double leaderSpeed) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    return (
               -vars->flatbedKa * egoAcceleration +
               vars->flatbedKv * (predSpeed - egoSpeed) +
               vars->flatbedKp * (gap2pred - vars->flatbedD - vars->flatbedH * (egoSpeed - leaderSpeed))
           );
}

double
MSCFModel_CC::getCACCConstantSpacing(const MSVehicle* veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    return vars->caccSpacing;
}

void
MSCFModel_CC::getVehicleInformation(const MSVehicle* veh, double& speed, double& acceleration, double& controllerAcceleration, Position& position, double& time) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    speed = veh->getSpeed();
    acceleration = veh->getAcceleration();
    controllerAcceleration = vars->controllerAcceleration;
    position = veh->getPosition();
    time = STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep());
}

void MSCFModel_CC::setParameter(MSVehicle* veh, const std::string& key, const std::string& value) const {
    // vehicle variables used to set the parameter
    CC_VehicleVariables* vars;

    ParBuffer buf(value);

    vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    try {
        if (key.compare(PAR_LEADER_SPEED_AND_ACCELERATION) == 0) {
            double x, y, vx, vy;
            buf >> vars->leaderSpeed >> vars->leaderAcceleration >> x >> y >> vars->leaderDataReadTime
                >> vars->leaderControllerAcceleration >> vx >> vy >> vars->leaderAngle;
            vars->leaderPosition = Position(x, y);
            vars->leaderVelocity = Position(vx, vy);
            vars->leaderInitialized = true;
            if (vars->frontInitialized) {
                vars->caccInitialized = true;
            }
            return;
        }
        if (key.compare(PAR_PRECEDING_SPEED_AND_ACCELERATION) == 0) {
            double x, y, vx, vy;
            buf >> vars->frontSpeed >> vars->frontAcceleration >> x >> y >> vars->frontDataReadTime
                >> vars->frontControllerAcceleration >> vx >> vy >> vars->frontAngle;
            vars->frontPosition = Position(x, y);
            vars->frontVelocity = Position(vx, vy);
            vars->frontInitialized = true;
            if (vars->leaderInitialized) {
                vars->caccInitialized = true;
            }
            return;
        }
        if (key.compare(CC_PAR_VEHICLE_DATA) == 0) {
            struct Plexe::VEHICLE_DATA vehicle;
            buf >> vehicle.index >> vehicle.speed >> vehicle.acceleration >>
                vehicle.positionX >> vehicle.positionY >> vehicle.time >>
                vehicle.length >> vehicle.u >> vehicle.speedX >>
                vehicle.speedY >> vehicle.angle;
            //if the index is larger than the number of cars, simply ignore the data
            if (vehicle.index >= vars->nCars || vehicle.index == -1) {
                return;
            }
            vars->vehicles[vehicle.index] = vehicle;
            if (!vars->initialized[vehicle.index] && vehicle.index != vars->position) {
                vars->nInitialized++;
            }
            vars->initialized[vehicle.index] = true;
            return;
        }
        if (key.compare(PAR_LEADER_FAKE_DATA) == 0) {
            buf >> vars->fakeData.leaderSpeed >> vars->fakeData.leaderAcceleration
                >> vars->fakeData.leaderControllerAcceleration;
            if (buf.last_empty()) {
                vars->useControllerAcceleration = false;
            }
            return;
        }
        if (key.compare(PAR_FRONT_FAKE_DATA) == 0) {
            buf >> vars->fakeData.frontSpeed >> vars->fakeData.frontAcceleration >> vars->fakeData.frontDistance
                >> vars->fakeData.frontControllerAcceleration;
            if (buf.last_empty()) {
                vars->useControllerAcceleration = false;
            }
            return;
        }
        if (key.compare(CC_PAR_VEHICLE_POSITION) == 0) {
            vars->position = StringUtils::toInt(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_PLATOON_SIZE) == 0) {
            vars->nCars = StringUtils::toInt(value.c_str());
            // given that we have a static matrix, check that we're not
            // setting a number of cars larger than the size of that matrix
            if (vars->nCars > MAX_N_CARS) {
                vars->nCars = MAX_N_CARS;
                std::stringstream warn;
                warn << "MSCFModel_CC: setting a number of cars of " << vars->nCars << " out of a maximum of " << MAX_N_CARS <<
                     ". The CONSENSUS controller will not work properly if chosen. If you are using a different controller " <<
                     "you can ignore this warning";
                WRITE_WARNING(warn.str());
            }
            return;
        }
        if (key.compare(PAR_ADD_MEMBER) == 0) {
            std::string id;
            int position;
            buf >> id >> position;
            vars->members[position] = id;
            return;
        }
        if (key.compare(PAR_REMOVE_MEMBER) == 0) {
            for (auto item = vars->members.begin(); item != vars->members.end(); item++)
                if (item->second.compare(value) == 0) {
                    vars->members.erase(item);
                    break;
                }
            return;
        }
        if (key.compare(PAR_ENABLE_AUTO_LANE_CHANGE) == 0) {
            vars->autoLaneChange = StringUtils::toInt(value.c_str()) == 1;
            return;
        }
        if (key.compare(CC_PAR_CACC_XI) == 0) {
            vars->caccXi = StringUtils::toDouble(value.c_str());
            recomputeParameters(veh);
            return;
        }
        if (key.compare(CC_PAR_CACC_OMEGA_N) == 0) {
            vars->caccOmegaN = StringUtils::toDouble(value.c_str());
            recomputeParameters(veh);
            return;
        }
        if (key.compare(CC_PAR_CACC_C1) == 0) {
            vars->caccC1 = StringUtils::toDouble(value.c_str());
            recomputeParameters(veh);
            return;
        }
        if (key.compare(CC_PAR_ENGINE_TAU) == 0) {
            vars->engineTau = StringUtils::toDouble(value.c_str());
            vars->engine->setParameter(FOLM_PAR_TAU, vars->engineTau);
            recomputeParameters(veh);
            vars->engine->setParameter(FOLM_PAR_TAU, vars->engineTau);
        }
        if (key.compare(CC_PAR_UMIN) == 0) {
            vars->uMin = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_UMAX) == 0) {
            vars->uMax = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_PLOEG_H) == 0) {
            vars->ploegH = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_PLOEG_KP) == 0) {
            vars->ploegKp = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_PLOEG_KD) == 0) {
            vars->ploegKd = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_FLATBED_KA) == 0) {
            vars->flatbedKa = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_FLATBED_KV) == 0) {
            vars->flatbedKv = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_FLATBED_KP) == 0) {
            vars->flatbedKp = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_FLATBED_H) == 0) {
            vars->flatbedH = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_FLATBED_D) == 0) {
            vars->flatbedD = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(CC_PAR_VEHICLE_ENGINE_MODEL) == 0) {
            if (vars->engine) {
                delete vars->engine;
            }
            int engineModel = StringUtils::toInt(value.c_str());
            switch (engineModel) {
                case CC_ENGINE_MODEL_REALISTIC: {
                    vars->engine = new RealisticEngineModel();
                    vars->engine->setParameter(ENGINE_PAR_DT, TS);
                    veh->getInfluencer().setSpeedMode(0);
                    vars->engineModel = CC_ENGINE_MODEL_REALISTIC;
                    break;
                }
                case CC_ENGINE_MODEL_FOLM:
                default: {
                    vars->engine = new FirstOrderLagModel();
                    vars->engine->setParameter(FOLM_PAR_DT, TS);
                    vars->engine->setParameter(FOLM_PAR_TAU, vars->engineTau);
                    vars->engineModel = CC_ENGINE_MODEL_FOLM;
                    break;
                }
            }
            vars->engine->setMaximumAcceleration(myAccel);
            vars->engine->setMaximumDeceleration(myDecel);
            return;
        }
        if (key.compare(CC_PAR_VEHICLE_MODEL) == 0) {
            vars->engine->setParameter(ENGINE_PAR_VEHICLE, value);
            return;
        }
        if (key.compare(CC_PAR_VEHICLES_FILE) == 0) {
            vars->engine->setParameter(ENGINE_PAR_XMLFILE, value);
            return;
        }
        if (key.compare(PAR_CACC_SPACING) == 0) {
            vars->caccSpacing = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(PAR_FIXED_ACCELERATION) == 0) {
            buf >> vars->useFixedAcceleration >> vars->fixedAcceleration;
            return;
        }
        if (key.compare(PAR_CC_DESIRED_SPEED) == 0) {
            vars->ccDesiredSpeed = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(PAR_ACTIVE_CONTROLLER) == 0) {
            vars->activeController = (enum Plexe::ACTIVE_CONTROLLER) StringUtils::toInt(value.c_str());
            return;
        }
        if (key.compare(PAR_ACC_HEADWAY_TIME) == 0) {
            vars->accHeadwayTime = StringUtils::toDouble(value.c_str());
            return;
        }
        if (key.compare(PAR_USE_CONTROLLER_ACCELERATION) == 0) {
            vars->useControllerAcceleration = StringUtils::toInt(value.c_str()) != 0;
            return;
        }
        if (key.compare(PAR_USE_AUTO_FEEDING) == 0) {
            int af;
            std::string leaderId, frontId;
            buf >> af;
            vars->autoFeed = af == 1;
            if (vars->autoFeed) {
                vars->usePrediction = false;
                buf >> leaderId;
                if (buf.last_empty()) {
                    throw InvalidArgument("Trying to enable auto feeding without providing leader vehicle id");
                }
                vars->leaderVehicle = dynamic_cast<MSVehicle*>(MSNet::getInstance()->getVehicleControl().getVehicle(leaderId));
                if (vars->leaderVehicle == 0) {
                    throw libsumo::TraCIException("Vehicle '" + leaderId + "' is not known");
                }
                buf >> frontId;
                if (buf.last_empty()) {
                    throw InvalidArgument("Trying to enable auto feeding without providing front vehicle id");
                }
                vars->frontVehicle = dynamic_cast<MSVehicle*>(MSNet::getInstance()->getVehicleControl().getVehicle(frontId));
                if (vars->frontVehicle == 0) {
                    throw libsumo::TraCIException("Vehicle '" + frontId + "' is not known");
                }
                vars->leaderInitialized = true;
                vars->frontInitialized = true;
                vars->caccInitialized = true;
            }
            return;
        }
        if (key.compare(PAR_USE_PREDICTION) == 0) {
            vars->usePrediction = StringUtils::toInt(value.c_str()) == 1;
            return;
        }
    } catch (NumberFormatException&) {
        throw InvalidArgument("Invalid value '" + value + "' for parameter '" + key + "' for vehicle '" + veh->getID() + "'");
    }

}

std::string MSCFModel_CC::getParameter(const MSVehicle* veh, const std::string& key) const {
    // vehicle variables used to set the parameter
    CC_VehicleVariables* vars;
    ParBuffer buf;

    vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    if (key.compare(PAR_SPEED_AND_ACCELERATION) == 0) {
        Position velocity = veh->getVelocityVector();
        buf << veh->getSpeed() << veh->getAcceleration() <<
            vars->controllerAcceleration << veh->getPosition().x() <<
            veh->getPosition().y() <<
            STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep()) <<
            velocity.x() << velocity.y() << veh->getAngle();
        return buf.str();
    }
    if (key.compare(PAR_CRASHED) == 0) {
        return vars->crashed ? "1" : "0";
    }
    if (key.compare(PAR_RADAR_DATA) == 0) {
        double distance, relSpeed;
        getRadarMeasurements(veh, distance, relSpeed);
        buf << distance << relSpeed;
        return buf.str();
    }
    if (key.compare(PAR_LANES_COUNT) == 0) {
        buf << veh->getLane()->getEdge().getLanes().size();
        return buf.str();
    }
    if (key.compare(PAR_DISTANCE_TO_END) == 0) {
        //route of the vehicle
        const MSRoute* route;
        //edge the vehicle is currently traveling on
        const MSEdge* currentEdge;
        //last edge of the route of this vehicle
        const MSEdge* lastEdge;
        //current position of the vehicle on the edge its traveling in
        double positionOnEdge;
        //distance to trip end using
        double distanceToEnd;

        route = &veh->getRoute();
        currentEdge = veh->getEdge();
        lastEdge = route->getEdges().back();
        positionOnEdge = veh->getPositionOnLane();
        distanceToEnd = route->getDistanceBetween(positionOnEdge, lastEdge->getLanes()[0]->getLength(), currentEdge, lastEdge);

        buf << distanceToEnd;
        return buf.str();
    }
    if (key.compare(PAR_DISTANCE_FROM_BEGIN) == 0) {
        //route of the vehicle
        const MSRoute* route;
        //edge the vehicle is currently traveling on
        const MSEdge* currentEdge;
        //last edge of the route of this vehicle
        const MSEdge* firstEdge;
        //current position of the vehicle on the edge its traveling in
        double positionOnEdge;
        //distance to trip end using
        double distanceFromBegin;

        route = &veh->getRoute();
        currentEdge = veh->getEdge();
        firstEdge = route->getEdges().front();
        positionOnEdge = veh->getPositionOnLane();
        distanceFromBegin = route->getDistanceBetween(0, positionOnEdge, firstEdge, currentEdge);

        buf << distanceFromBegin;
        return buf.str();
    }
    if (key.compare(PAR_CC_DESIRED_SPEED) == 0) {
        buf << (double)vars->ccDesiredSpeed;
        return buf.str();
    }
    if (key.compare(PAR_ACTIVE_CONTROLLER) == 0) {
        buf << (int)vars->activeController;
        return buf.str();
    }
    if (key.compare(PAR_ACC_HEADWAY_TIME) == 0) {
        buf << (double)vars->accHeadwayTime;
        return buf.str();
    }
    if (key.compare(PAR_ACC_ACCELERATION) == 0) {
        buf << getACCAcceleration(veh);
        return buf.str();
    }
    if (key.compare(PAR_CACC_SPACING) == 0) {
        buf << vars->caccSpacing;
        return buf.str();
    }
    if (key.find(CC_PAR_VEHICLE_DATA) == 0) {
        ParBuffer inBuf(key);
        int index;
        inBuf >> index;
        struct Plexe::VEHICLE_DATA vehicle;
        if (index >= vars->nCars || index < 0) {
            vehicle.index = -1;
        } else {
            vehicle = vars->vehicles[index];
        }
        buf << vehicle.index << vehicle.speed << vehicle.acceleration <<
            vehicle.positionX << vehicle.positionY << vehicle.time <<
            vehicle.length << vehicle.u << vehicle.speedX <<
            vehicle.speedY << vehicle.angle;
        return buf.str();
    }
    if (key.compare(PAR_ENGINE_DATA) == 0) {
        int gear;
        double rpm;
        RealisticEngineModel* engine = dynamic_cast<RealisticEngineModel*>(vars->engine);
        if (engine) {
            engine->getEngineData(veh->getSpeed(), gear, rpm);
        } else {
            gear = -1;
            rpm = 0;
        }
        buf << (gear + 1) << rpm;
        return buf.str();
    }
    return "";
}

void MSCFModel_CC::recomputeParameters(const MSVehicle* veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    vars->caccAlpha1 = 1 - vars->caccC1;
    vars->caccAlpha2 = vars->caccC1;
    vars->caccAlpha3 = -(2 * vars->caccXi - vars->caccC1 * (vars->caccXi + sqrt(vars->caccXi * vars->caccXi - 1))) * vars->caccOmegaN;
    vars->caccAlpha4 = -(vars->caccXi + sqrt(vars->caccXi * vars->caccXi - 1)) * vars->caccOmegaN * vars->caccC1;
    vars->caccAlpha5 = -vars->caccOmegaN * vars->caccOmegaN;
}

void MSCFModel_CC::resetConsensus(const MSVehicle* veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    for (int i = 0; i < MAX_N_CARS; i++) {
        vars->initialized[i] = false;
        vars->nInitialized = 0;
    }
}

void MSCFModel_CC::switchOnACC(const MSVehicle* veh, double ccDesiredSpeed)  const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    vars->ccDesiredSpeed = ccDesiredSpeed;
    vars->activeController = Plexe::ACC;
}

enum Plexe::ACTIVE_CONTROLLER MSCFModel_CC::getActiveController(const MSVehicle* veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    return vars->activeController;
}

void MSCFModel_CC::getRadarMeasurements(const MSVehicle* veh, double& distance, double& relativeSpeed) const {
    std::pair<std::string, double> l = libsumo::Vehicle::getLeader(veh->getID(), 250);
    if (l.second < 0) {
        distance = -1;
        relativeSpeed = 0;
    } else {
        distance = l.second;
        SUMOVehicle* leader = MSNet::getInstance()->getVehicleControl().getVehicle(l.first);
        relativeSpeed = leader->getSpeed() - veh->getSpeed();
    }
}

double MSCFModel_CC::getACCAcceleration(const MSVehicle* veh) const {
    CC_VehicleVariables* vars = (CC_VehicleVariables*) veh->getCarFollowVariables();
    double distance, relSpeed;
    getRadarMeasurements(veh, distance, relSpeed);
    if (distance < 0) {
        return 0;
    } else {
        return _acc(veh, veh->getSpeed(), relSpeed + veh->getSpeed(), distance, vars->accHeadwayTime);
    }
}

int MSCFModel_CC::getMyLanesCount() const {
    return myLanesCount;
}

MSCFModel*
MSCFModel_CC::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_CC(vtype);
}
