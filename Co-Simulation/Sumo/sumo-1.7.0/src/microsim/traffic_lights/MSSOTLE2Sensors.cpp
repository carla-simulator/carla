/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSSOTLE2Sensors.cpp
/// @author  Gianfilippo Slager
/// @author  Alessio Bonfietti
/// @author  Federico Caselli
/// @date    2010-02-25
///
// The class for SOTL sensors of "E2" type
/****************************************************************************/

#include <utils/common/StringUtils.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicleType.h>
#include "MSSOTLE2Sensors.h"

#define INVALID_POSITION std::numeric_limits<double>::max()

MSSOTLE2Sensors::MSSOTLE2Sensors(std::string tlLogicID,
                                 const MSTrafficLightLogic::Phases* phases) :
    MSSOTLSensors(tlLogicID, phases) {
}

MSSOTLE2Sensors::~MSSOTLE2Sensors(void) {
    //Deleting sensors takes place in the detector control
}

void MSSOTLE2Sensors::buildSensors(
    MSTrafficLightLogic::LaneVectorVector controlledLanes,
    NLDetectorBuilder& nb) {
    buildSensors(controlledLanes, nb, INPUT_SENSOR_LENGTH);
}

void MSSOTLE2Sensors::buildSensors(
    MSTrafficLightLogic::LaneVectorVector controlledLanes,
    NLDetectorBuilder& nb, double sensorLength) {
    //for each lane build an appropriate sensor on it
    MSLane* currentLane = nullptr;

    //input and ouput lanes
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector =
                controlledLanes.begin(); laneVector != controlledLanes.end();
            laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane =
                    laneVector->begin(); lane != laneVector->end(); lane++) {
            currentLane = (*lane);
            buildSensorForLane(currentLane, nb, sensorLength);
        }
    }
}
/****************************************************************************/
/*
 * Count Sensors. Should be refactor to make a new class.
 */
void MSSOTLE2Sensors::buildCountSensors(
    MSTrafficLightLogic::LaneVectorVector controlledLanes,
    NLDetectorBuilder& nb) {
    //for each lane build an appropriate sensor on it
    MSLane* currentLane = nullptr;
    //input and ouput lanes
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector =
                controlledLanes.begin(); laneVector != controlledLanes.end();
            laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane =
                    laneVector->begin(); lane != laneVector->end(); lane++) {
            currentLane = (*lane);
            buildCountSensorForLane(currentLane, nb);
        }
    }
}

void MSSOTLE2Sensors::buildCountOutSensors(
    MSTrafficLightLogic::LaneVectorVector controlledLanes,
    NLDetectorBuilder& nb) {
    //for each lane build an appropriate sensor on it
    MSLane* currentLane = nullptr;

    //input and ouput lanes
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector =
                controlledLanes.begin(); laneVector != controlledLanes.end();
            laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane =
                    laneVector->begin(); lane != laneVector->end(); lane++) {
            currentLane = (*lane);
            buildCountSensorForOutLane(currentLane, nb);
        }
    }
}

void MSSOTLE2Sensors::buildOutSensors(
    MSTrafficLightLogic::LaneVectorVector controlledLanes,
    NLDetectorBuilder& nb) {
    buildOutSensors(controlledLanes, nb, OUTPUT_SENSOR_LENGTH);
}
void MSSOTLE2Sensors::buildOutSensors(
    MSTrafficLightLogic::LaneVectorVector controlledLanes,
    NLDetectorBuilder& nb, double sensorLength) {
    //for each lane build an appropriate sensor on it
    MSLane* currentLane = nullptr;

    //input and ouput lanes
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector =
                controlledLanes.begin(); laneVector != controlledLanes.end();
            laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane =
                    laneVector->begin(); lane != laneVector->end(); lane++) {
            currentLane = (*lane);
            buildSensorForOutLane(currentLane, nb, sensorLength);
        }
    }
}

void MSSOTLE2Sensors::buildCountSensorForLane(MSLane* lane, NLDetectorBuilder& nb) {
    double sensorPos;
    double lensorLength;
    MSE2Collector* newSensor = nullptr;
    //Check not to have more than a sensor for lane
    if (m_sensorMap.find(lane->getID()) == m_sensorMap.end()) {

        //Check and set zero if the lane is not long enough for the specified sensor start
        sensorPos = COUNT_SENSOR_START <= lane->getLength() ? COUNT_SENSOR_START : 0;

        //Original:
        double sensorLength = INPUT_COUNT_SENSOR_LENGTH;

        //Check and trim if the lane is not long enough for the specified sensor length
        lensorLength = sensorLength <= (lane->getLength() - sensorPos) ? sensorLength : (lane->getLength() - sensorPos);

        //TODO check this lengths
//        DBG(
//            std::ostringstream phero_str;
//            phero_str << " lane " << lane->getID() << " sensorPos= " << sensorPos
//            << " ,SENSOR_START  " << SENSOR_START << "; lane->getLength = "
//            << lane->getLength() << " ,lensorLength= " << lensorLength
//            << " ,SENSOR_LENGTH= " << INPUT_SENSOR_LENGTH;
//            WRITE_MESSAGE(
//                "MSSOTLE2Sensors::buildSensorForLane::" + phero_str.str());
//        )

        //Create sensor for lane and insert it into the map<MSLane*, MSE2Collector*>
        newSensor = nb.createE2Detector(
                        "SOTL_E2_lane:" + lane->getID() + "_tl:" + tlLogicID,
                        DU_TL_CONTROL, lane,
                        (lane->getLength() - sensorPos - lensorLength), INVALID_POSITION, lensorLength,
                        HALTING_TIME_THRS, HALTING_SPEED_THRS, DIST_THRS, "");

        MSNet::getInstance()->getDetectorControl().add(
            SUMO_TAG_LANE_AREA_DETECTOR, newSensor);

        m_sensorMap.insert(MSLaneID_MSE2Collector(lane->getID(), newSensor));
        m_maxSpeedMap.insert(MSLaneID_MaxSpeed(lane->getID(), lane->getSpeedLimit()));
    }
}

void MSSOTLE2Sensors::buildCountSensorForOutLane(MSLane* lane, NLDetectorBuilder& nb) {
    double sensorPos;
    double lensorLength;
    MSE2Collector* newSensor = nullptr;
    //Check not to have more than a sensor for lane
    if (m_sensorMap.find(lane->getID()) == m_sensorMap.end()) {

        //Original:
        double sensorLength = OUTPUT_COUNT_SENSOR_LENGTH;
        //Check and set zero if the lane is not long enough for the specified sensor start
        sensorPos = (lane->getLength() - sensorLength)
                    - (SENSOR_START <= lane->getLength() ? SENSOR_START : 0);

        //Check and trim if the lane is not long enough for the specified sensor length
        lensorLength =
            sensorLength <= (lane->getLength() - sensorPos) ?
            sensorLength : (lane->getLength() - sensorPos);

        //TODO check this lengths
//        DBG(
//            std::ostringstream phero_str;
//            phero_str << " lane " << lane->getID() << " sensorPos= " << sensorPos
//            << " ,SENSOR_START  " << SENSOR_START << "; lane->getLength = "
//            << lane->getLength() << " ,lensorLength= " << lensorLength
//            << " ,SENSOR_LENGTH= " << INPUT_SENSOR_LENGTH;
//            WRITE_MESSAGE(
//                "MSSOTLE2Sensors::buildSensorForLane::" + phero_str.str());
//        )

        //Create sensor for lane and insert it into the map<MSLane*, MSE2Collector*>
        newSensor = nb.createE2Detector(
                        "SOTL_E2_lane:" + lane->getID() + "_tl:" + tlLogicID,
                        DU_TL_CONTROL, lane,
                        (lane->getLength() - sensorPos - lensorLength), INVALID_POSITION, lensorLength,
                        HALTING_TIME_THRS, HALTING_SPEED_THRS, DIST_THRS, "");

        MSNet::getInstance()->getDetectorControl().add(
            SUMO_TAG_LANE_AREA_DETECTOR, newSensor);

        m_sensorMap.insert(MSLaneID_MSE2Collector(lane->getID(), newSensor));
        m_maxSpeedMap.insert(MSLaneID_MaxSpeed(lane->getID(), lane->getSpeedLimit()));
    }
}

int MSSOTLE2Sensors::getPassedVeh(std::string laneId, bool /* out */) {
    MSLaneID_MSE2CollectorMap::const_iterator sensorsIterator;
    sensorsIterator = m_sensorMap.find(laneId);
    if (sensorsIterator == m_sensorMap.end()) {
        assert(0);
        return 0;
    } else {
        int additional = 0;
        if (m_continueSensorOnLanes.find(laneId) != m_continueSensorOnLanes.end())
            for (std::vector<std::string>::iterator it = m_continueSensorOnLanes[laneId].begin(); it != m_continueSensorOnLanes[laneId].end(); ++ it) {
                int tmp = 0;
                if (getVelueFromSensor(*it, &MSE2Collector::getPassedVeh, tmp)) {
                    additional += tmp;
                }
            }
        return sensorsIterator->second->getPassedVeh() + additional;
    }
}

void
MSSOTLE2Sensors::subtractPassedVeh(std::string laneId, int passed) {
    //TODO Ivan: ciclare tutti i sensori di count e resettare passedVeh
    MSLaneID_MSE2CollectorMap::const_iterator sensorsIterator;

    sensorsIterator = m_sensorMap.find(laneId);
    if (sensorsIterator != m_sensorMap.end()) {
        sensorsIterator->second->subtractPassedVeh(passed);
    }
}

/*******************************************************************************/

void MSSOTLE2Sensors::buildSensorForLane(MSLane* lane, NLDetectorBuilder& nb) {
    buildSensorForLane(lane, nb, INPUT_SENSOR_LENGTH);
}



void MSSOTLE2Sensors::buildSensorForLane(MSLane* lane, NLDetectorBuilder& nb, double sensorLength) {
    const MSEdge* edge = &lane->getEdge();
    if (edge->isInternal() || edge->isWalkingArea() || edge->isCrossing()) {
        return;
    }
    double sensorPos;
    double lensorLength;
    MSE2Collector* newSensor = nullptr;
    //Check not to have more than a sensor for lane
    if (m_sensorMap.find(lane->getID()) == m_sensorMap.end()) {

        //Check and set zero if the lane is not long enough for the specified sensor start
        sensorPos = SENSOR_START <= lane->getLength() ? SENSOR_START : 0;
        //Check and trim if the lane is not long enough for the specified sensor length
        lensorLength = sensorLength <= (lane->getLength() - sensorPos) ? sensorLength : (lane->getLength() - sensorPos);
        //TODO check this lengths
//        DBG(
//            std::ostringstream phero_str;
//            phero_str << " lane " << lane->getID() << " sensorPos= " << sensorPos
//            << " ,SENSOR_START  " << SENSOR_START << "; lane->getLength = "
//            << lane->getLength() << " ,lensorLength= " << lensorLength
//            << " ,SENSOR_LENGTH= " << INPUT_SENSOR_LENGTH;
//            WRITE_MESSAGE(
//                "MSSOTLE2Sensors::buildSensorForLane::" + phero_str.str());
//        )

        //Create sensor for lane and insert it into the map<MSLane*, MSE2Collector*>
        newSensor = nb.createE2Detector(
                        "SOTL_E2_lane:" + lane->getID() + "_tl:" + tlLogicID,
                        DU_TL_CONTROL, lane,
                        (lane->getLength() - sensorPos - lensorLength), INVALID_POSITION, lensorLength,
                        HALTING_TIME_THRS, HALTING_SPEED_THRS, DIST_THRS, "");

        MSNet::getInstance()->getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, newSensor);

        m_sensorMap.insert(MSLaneID_MSE2Collector(lane->getID(), newSensor));
        m_maxSpeedMap.insert(MSLaneID_MaxSpeed(lane->getID(), lane->getSpeedLimit()));
        //Avoid creating really small sensors. Only add a further sensors if the one created is less than 90% the requested length
        if (lensorLength < sensorLength * 0.9) {
            std::ostringstream oss;
            oss << "Sensor on lane " << lane->getID() << " is long " << lensorLength << ", while it should be " << sensorLength << ". Continuing it on the other lanes if possible";
            WRITE_MESSAGE(oss.str())
            for (std::vector<MSLane::IncomingLaneInfo>::const_iterator it = lane->getIncomingLanes().begin(); it != lane->getIncomingLanes().end(); ++it) {
                const MSEdge* edge = &it->lane->getEdge();
                if (!edge->isInternal() && !edge->isWalkingArea() && !edge->isCrossing()) {
                    buildContinueSensior(lane, nb, sensorLength, it->lane, lensorLength);
                }
            }
        }
    }
}

void MSSOTLE2Sensors::buildContinueSensior(MSLane* lane, NLDetectorBuilder& nb, double sensorLength, MSLane* continueOnLane, double usedLength) {
    double availableLength = sensorLength - usedLength;
    if (m_sensorMap.find(continueOnLane->getID()) == m_sensorMap.end()) {
        double length = availableLength <= continueOnLane->getLength() ? availableLength : continueOnLane->getLength();
        MSE2Collector* newSensor = nb.createE2Detector(
                                       "SOTL_E2_lane:" + continueOnLane->getID() + "_tl:" + tlLogicID,
                                       DU_TL_CONTROL, continueOnLane,
                                       (continueOnLane->getLength() - length), INVALID_POSITION, length,
                                       HALTING_TIME_THRS, HALTING_SPEED_THRS, DIST_THRS, "");
        MSNet::getInstance()->getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, newSensor);
        m_sensorMap.insert(MSLaneID_MSE2Collector(continueOnLane->getID(), newSensor));
        m_continueSensorOnLanes[lane->getID()].push_back(continueOnLane->getID());
        std::ostringstream oss;
        oss << "Continue sensor on lane " << continueOnLane->getID() << ". Current length " << (length + usedLength);
        WRITE_MESSAGE(oss.str())
        //Continue other line if needed.
        if (length + usedLength < sensorLength * 0.9) {
            for (std::vector<MSLane::IncomingLaneInfo>::const_iterator it = continueOnLane->getIncomingLanes().begin(); it != continueOnLane->getIncomingLanes().end(); ++it) {
                const MSEdge* edge = &it->lane->getEdge();
                if (!edge->isInternal() && !edge->isWalkingArea() && !edge->isCrossing()) {
                    buildContinueSensior(lane, nb, sensorLength, it->lane, length + usedLength);
                }
            }
        }
    }
}

void MSSOTLE2Sensors::buildSensorForOutLane(MSLane* lane,
        NLDetectorBuilder& nb) {
    buildSensorForOutLane(lane, nb, OUTPUT_SENSOR_LENGTH);
}

void MSSOTLE2Sensors::buildSensorForOutLane(MSLane* lane,
        NLDetectorBuilder& nb, double sensorLength) {
    double sensorPos;
    double lensorLength;
    MSE2Collector* newSensor = nullptr;
    //Check not to have more than a sensor for lane
    if (m_sensorMap.find(lane->getID()) == m_sensorMap.end()) {

        //Original:
        //double sensorLength = OUTPUT_SENSOR_LENGTH;
        //Check and set zero if the lane is not long enough for the specified sensor start
        sensorPos = (lane->getLength() - sensorLength)
                    - (SENSOR_START <= lane->getLength() ? SENSOR_START : 0);

        //Check and trim if the lane is not long enough for the specified sensor length
        lensorLength =
            sensorLength <= (lane->getLength() - sensorPos) ?
            sensorLength : (lane->getLength() - sensorPos);

        //TODO check this lengths
//        DBG(
//            std::ostringstream phero_str;
//            phero_str << " lane " << lane->getID() << " sensorPos= " << sensorPos
//            << " ,SENSOR_START  " << SENSOR_START << "; lane->getLength = "
//            << lane->getLength() << " ,lensorLength= " << lensorLength
//            << " ,SENSOR_LENGTH= " << INPUT_SENSOR_LENGTH;
//            WRITE_MESSAGE(
//                "MSSOTLE2Sensors::buildSensorForLane::" + phero_str.str());
//        )

        //Create sensor for lane and insert it into the map<MSLane*, MSE2Collector*>
        newSensor = nb.createE2Detector(
                        "SOTL_E2_lane:" + lane->getID() + "_tl:" + tlLogicID,
                        DU_TL_CONTROL, lane,
                        (lane->getLength() - sensorPos - lensorLength), INVALID_POSITION, lensorLength,
                        HALTING_TIME_THRS, HALTING_SPEED_THRS, DIST_THRS, "");

        MSNet::getInstance()->getDetectorControl().add(
            SUMO_TAG_LANE_AREA_DETECTOR, newSensor);

        m_sensorMap.insert(MSLaneID_MSE2Collector(lane->getID(), newSensor));
        m_maxSpeedMap.insert(MSLaneID_MaxSpeed(lane->getID(), lane->getSpeedLimit()));
    }
}

int MSSOTLE2Sensors::countVehicles(MSLane* lane) {
    return countVehicles(lane->getID());
}

/*
 * Estimate queue length according to the distance of the last vehicles
 */

double MSSOTLE2Sensors::getEstimateQueueLength(std::string laneId) {
    MSLaneID_MSE2CollectorMap::const_iterator sensorsIterator = m_sensorMap.find(laneId);
    if (sensorsIterator == m_sensorMap.end()) {
        assert(0);
        return 0;
    } else {
        double estQL = sensorsIterator->second->getEstimateQueueLength();
        if (estQL == -1) {
            return 0;
        } else
//            DBG(
//                std::ostringstream str;
//                str << "MSSOTLE2Sensors::getEstimateQueueLength lane " << sensorsIterator->second->getLane()->getID()
//                << " laneLength " << sensorsIterator->second->getLane()->getLength() << " estimateQueueLength " <<  estQL;
//                WRITE_MESSAGE(str.str());
//            )
        {
            return estQL;
        }
    }
}

/*
 * Estimate queue length according to the distance of the last vehicles that exceed a threshold
 */

int MSSOTLE2Sensors::estimateVehicles(std::string laneId) {
    MSLaneID_MSE2CollectorMap::const_iterator sensorsIterator = m_sensorMap.find(laneId);
    if (sensorsIterator == m_sensorMap.end()) {
        assert(0);
        return 0;
    }
    int additional = 0;
    if (m_continueSensorOnLanes.find(laneId) != m_continueSensorOnLanes.end())
        for (std::vector<std::string>::iterator it = m_continueSensorOnLanes[laneId].begin(); it != m_continueSensorOnLanes[laneId].end(); ++ it) {
            if (m_sensorMap.find(*it) != m_sensorMap.end()) {
                additional += m_sensorMap[*it]->getEstimatedCurrentVehicleNumber(speedThresholdParam);
            }
        }
    return sensorsIterator->second->getEstimatedCurrentVehicleNumber(speedThresholdParam) + additional;
}

int MSSOTLE2Sensors::countVehicles(std::string laneId) {
    MSLaneID_MSE2CollectorMap::const_iterator sensorsIterator = m_sensorMap.find(laneId);
    if (sensorsIterator == m_sensorMap.end()) {
        assert(0);
        return 0;
    }
    int additional = 0;
    if (m_continueSensorOnLanes.find(laneId) != m_continueSensorOnLanes.end()) {
        for (std::vector<std::string>::iterator it = m_continueSensorOnLanes[laneId].begin(); it != m_continueSensorOnLanes[laneId].end(); ++ it) {
            if (m_sensorMap.find(*it) != m_sensorMap.end()) {
                additional += count(m_sensorMap[*it]);
            }
        }
    }
    return count(sensorsIterator->second) + additional;
}

double MSSOTLE2Sensors::getMaxSpeed(std::string laneId) {
    MSLaneID_MaxSpeedMap::const_iterator sensorsIteratorIn = m_maxSpeedMap.find(laneId);
    if (sensorsIteratorIn == m_maxSpeedMap.end()) {
        assert(0);
        WRITE_ERROR("MSSOTLE2Sensors::meanVehiclesSpeed:: No lane found " + laneId);
        return 0;
    }
    return sensorsIteratorIn->second;
}

double MSSOTLE2Sensors::meanVehiclesSpeed(MSLane* lane) {
    return meanVehiclesSpeed(lane->getID());
}

double MSSOTLE2Sensors::meanVehiclesSpeed(std::string laneId) {
    MSLaneID_MSE2CollectorMap::const_iterator sensorsIteratorOut = m_sensorMap.find(laneId);
    if (sensorsIteratorOut == m_sensorMap.end()) {
        assert(0);
        WRITE_ERROR("MSSOTLE2Sensors::meanVehiclesSpeed:: No lane found " + laneId);
        return 0;
    }
    double meanSpeedAcc = 0;
    int totalCarNumer = 0;
    if (m_continueSensorOnLanes.find(laneId) != m_continueSensorOnLanes.end())
        for (std::vector<std::string>::iterator it = m_continueSensorOnLanes[laneId].begin(); it != m_continueSensorOnLanes[laneId].end(); ++ it) {
            int number = 0;
            double mean = -1;
            if (!getVelueFromSensor(*it, &MSE2Collector::getCurrentVehicleNumber, number)) {
                continue;
            }
            totalCarNumer += number;
            getVelueFromSensor(*it, &MSE2Collector::getCurrentMeanSpeed, mean);
            meanSpeedAcc += mean * (double) number;
        }
    int number = sensorsIteratorOut->second->getCurrentVehicleNumber();
    totalCarNumer += number;
    double mean = sensorsIteratorOut->second->getCurrentMeanSpeed();
    meanSpeedAcc += mean * (double) number;
    return totalCarNumer == 0 ? -1 : meanSpeedAcc / (double) totalCarNumer;
}

std::string trim(std::string& str) {
    int first = (int)str.find_first_not_of(' ');
    int last = (int)str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

void MSSOTLE2Sensors::setVehicleWeigths(const std::string& weightString) {
    std::vector<std::string> types;
    split(weightString, ';', types);
    std::ostringstream logstr;
    logstr << "[MSSOTLE2Sensors::setVehicleWeigths] ";
    for (std::vector<std::string>::iterator typesIt = types.begin(); typesIt != types.end(); ++typesIt) {
        std::vector<std::string> typeWeight;
        split(*typesIt, '=', typeWeight);
        if (typeWeight.size() == 2) {
            std::string type = trim(typeWeight[0]);
            int value = StringUtils::toInt(typeWeight[1]);
            logstr << type << "=" << value << " ";
            m_typeWeightMap[type] = value;
        }
    }
    WRITE_MESSAGE(logstr.str());
}

int MSSOTLE2Sensors::count(MSE2Collector* sensor) {
    int totCars = sensor->getCurrentVehicleNumber();
    if (m_typeWeightMap.size() == 0) {
        return totCars;
    }
    int number = 0;
    const std::vector<MSE2Collector::VehicleInfo*> vehicles = sensor->getCurrentVehicles();
    std::ostringstream logstr;
    logstr << "[MSSOTLE2Sensors::count]";
    for (std::vector<MSE2Collector::VehicleInfo*>::const_iterator vit = vehicles.begin(); vit != vehicles.end(); ++vit) {
        if ((*vit)->onDetector) {
            const std::string vtype = (*vit)->type;
            if (m_typeWeightMap.find(vtype) != m_typeWeightMap.end()) {
                number += m_typeWeightMap[vtype];
//                DBG(logstr << " Added " << m_typeWeightMap[vtype] << " for vtype " << vtype;)
            } else {
                ++number;
            }
        }
    }
//    DBG(if (totCars != number) {
//    logstr << ". Real number " << totCars << "; weighted " << number;
//    WRITE_MESSAGE(logstr.str());
//    })
    return number;
}
