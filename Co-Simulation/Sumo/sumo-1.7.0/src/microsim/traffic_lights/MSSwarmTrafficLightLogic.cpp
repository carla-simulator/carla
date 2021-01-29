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
/// @file    MSSwarmTrafficLightLogic.cpp
/// @author  Gianfilippo Slager
/// @author  Federico Caselli
/// @date    Mar 2010
///
// The class for Swarm-based logics
/****************************************************************************/

#include "MSSwarmTrafficLightLogic.h"
#include "../MSEdge.h"

#if 1
#define ANALYSIS_DBG(X) {X}
#else
#define ANALYSIS_DBG(X) DBG(X)
#endif

MSSwarmTrafficLightLogic::MSSwarmTrafficLightLogic(MSTLLogicControl& tlcontrol, const std::string& id,
        const std::string& programID, const Phases& phases, int step, SUMOTime delay,
        const std::map<std::string, std::string>& parameters) :
    MSSOTLHiLevelTrafficLightLogic(tlcontrol, id, programID, TrafficLightType::SWARM_BASED, phases, step, delay, parameters) {

    std::string pols = getPoliciesParam();
    std::transform(pols.begin(), pols.end(), pols.begin(), ::tolower);
    DBG(std::ostringstream str; str << "policies: " << pols; WRITE_MESSAGE(str.str());)

    if (pols.find("platoon") != std::string::npos) {
        addPolicy(new MSSOTLPlatoonPolicy(new MSSOTLPolicy5DFamilyStimulus("PLATOON", parameters), parameters));
    }
    if (pols.find("phase") != std::string::npos) {
        addPolicy(new MSSOTLPhasePolicy(new MSSOTLPolicy5DFamilyStimulus("PHASE", parameters), parameters));
    }
    if (pols.find("marching") != std::string::npos) {
        addPolicy(new MSSOTLMarchingPolicy(new MSSOTLPolicy5DFamilyStimulus("MARCHING", parameters), parameters));
    }
    if (pols.find("congestion") != std::string::npos) {
        addPolicy(new MSSOTLCongestionPolicy(new MSSOTLPolicy5DFamilyStimulus("CONGESTION", parameters), parameters));
    }

    if (getPolicies().empty()) {
        WRITE_ERROR("NO VALID POLICY LIST READ");
    }

    mustChange = false;
    skipEta = false;
    gotTargetLane = false;

    DBG(
        std::ostringstream d_str; d_str << getMaxCongestionDuration(); vector<MSSOTLPolicy*> policies = getPolicies();

    WRITE_MESSAGE("getMaxCongestionDuration " + d_str.str()); for (int i = 0; i < policies.size(); i++) {
    MSSOTLPolicy* policy = policies[i];
        MSSOTLPolicyDesirability* stim = policy->getDesirabilityAlgorithm();
        std::ostringstream _str;
        _str << policy->getName() << stim->getMessage() << " getThetaSensitivity " << policy->getThetaSensitivity() << " .";
        WRITE_MESSAGE(_str.str());
    })
    congestion_steps = 0;
    m_useVehicleTypesWeights = getParameter("USE_VEHICLE_TYPES_WEIGHTS", "0") == "1";
    if (m_useVehicleTypesWeights && pols.find("phase") == std::string::npos) {
        WRITE_ERROR("VEHICLE TYPES WEIGHT only works with phase policy, which is missing");
    }
}

MSSwarmTrafficLightLogic::~MSSwarmTrafficLightLogic() {
    if (logData && swarmLogFile.is_open()) {
        swarmLogFile.close();
    }
    for (std::map<std::string, CircularBuffer<double>*>::iterator it = m_meanSpeedHistory.begin();
            it != m_meanSpeedHistory.end(); ++it) {
        delete it->second;
    }
    m_meanSpeedHistory.clear();
    for (std::map<std::string, CircularBuffer<double>*>::iterator it = m_derivativeHistory.begin();
            it != m_derivativeHistory.end(); ++it) {
        delete it->second;
    }
    m_derivativeHistory.clear();
}

bool MSSwarmTrafficLightLogic::allowLine(MSLane* lane) {
    //No walking areas
    if (lane->getEdge().isWalkingArea()) {
        return false;
    }
    //No pedestrian crossing
    if (lane->getEdge().isCrossing()) {
        return false;
    }
    //No pedestrian only lanes
    if (lane->getPermissions() == SVC_PEDESTRIAN) {
        return false;
    }
    //No bicycle only lanes
    if (lane->getPermissions() == SVC_BICYCLE) {
        return false;
    }
    //No pedestrian and bicycle only lanes
    if (lane->getPermissions() == (SVC_PEDESTRIAN | SVC_BICYCLE)) {
        return false;
    }
    return true;
}

void MSSwarmTrafficLightLogic::init(NLDetectorBuilder& nb) {
    MSSOTLHiLevelTrafficLightLogic::init(nb);
    //Setting the startup policy
    choosePolicy(0, 0, 0, 0);
    //Initializing the random number generator to a time-dependent seed
    srand((int) time(nullptr));
    //Initializing pheromone maps according to input lanes
    //For each lane insert a pair into maps
    MSLane* currentLane = nullptr;

//	Derivative
    const int derivativeHistorySize = StringUtils::toInt(getParameter("PHERO_DERIVATIVE_HISTORY_SIZE", "3"));
    const int meanSpeedHistorySize = StringUtils::toInt(getParameter("PHERO_MEAN_SPEED_HISTORY_SIZE", "3"));
    m_derivativeAlpha = StringUtils::toDouble(getParameter("PHERO_DERIVATIVE_ALPHA", "1"));
    m_losCounter = 0;
    m_losMaxLimit = StringUtils::toInt(getParameter("LOSS_OF_SIGNAL_LIMIT", "10"));

    int index = 0;
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector = myLanes.begin();
            laneVector != myLanes.end(); laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane = laneVector->begin(); lane != laneVector->end();
                lane++) {
            currentLane = (*lane);
            if (pheromoneInputLanes.find(currentLane->getID()) == pheromoneInputLanes.end()) {
                laneCheck[currentLane] = false;
                if (allowLine(currentLane)) {
                    pheromoneInputLanes.insert(MSLaneId_Pheromone(currentLane->getID(), 0.0));
//					Consider the derivative only for the input lane
                    m_meanSpeedHistory.insert(std::make_pair(currentLane->getID(), new CircularBuffer<double>(meanSpeedHistorySize)));
                    m_derivativeHistory.insert(std::make_pair(currentLane->getID(), new CircularBuffer<double>(derivativeHistorySize)));
                    ANALYSIS_DBG(
                        WRITE_MESSAGE("MSSwarmTrafficLightLogic::init Intersection " + getID() + " pheromoneInputLanes adding " + currentLane->getID());)
                } else {
                    ANALYSIS_DBG(
                        WRITE_MESSAGE("MSSwarmTrafficLightLogic::init Intersection " + getID() + " pheromoneInputLanes: lane " + currentLane->getID() + " not allowed");)
                }
            }
            m_laneIndexMap[currentLane->getID()].push_back(index++);
        }
    }

    LinkVectorVector myLinks = getLinks();
    for (int i = 0; i < (int)myLinks.size(); i++) {
        LinkVector oneLink = getLinksAt(i);
        for (int j = 0; j < (int)oneLink.size(); j++) {
            currentLane = oneLink[j]->getLane();
            if (pheromoneOutputLanes.find(currentLane->getID()) == pheromoneOutputLanes.end()) {
                laneCheck[currentLane] = false;
                if (allowLine(currentLane)) {
                    pheromoneOutputLanes.insert(MSLaneId_Pheromone(currentLane->getID(), 0.0));
                    ANALYSIS_DBG(
                        WRITE_MESSAGE("MSSwarmTrafficLightLogic::init Intersection " + getID() + " pheromoneOutputLanes adding " + currentLane->getID());)
                } else {
                    ANALYSIS_DBG(
                        WRITE_MESSAGE("MSSwarmTrafficLightLogic::init Intersection " + getID() + " pheromoneOutputLanes lane " + currentLane->getID() + " not allowed");)
                }
            }
        }
    }

    initScaleFactorDispersionIn((int)pheromoneInputLanes.size());
    initScaleFactorDispersionOut((int)pheromoneOutputLanes.size());
    //Initializing thresholds for theta evaluations
    lastThetaSensitivityUpdate = MSNet::getInstance()->getCurrentTimeStep();

    WRITE_MESSAGE("*** Intersection " + getID() + " will run using MSSwarmTrafficLightLogic ***");
    std::string logFileName = getParameter("SWARMLOG", "");
    logData = logFileName.compare("") != 0;
    if (logData) {
        swarmLogFile.open(logFileName.c_str(), std::ios::out | std::ios::binary);
    }
//	Log the initial state
    ANALYSIS_DBG(
        WRITE_MESSAGE("TL " + getID() + " time 0 Policy: " + getCurrentPolicy()->getName() + " (pheroIn= 0 ,pheroOut= 0 ) OldPolicy: " + getCurrentPolicy()->getName() + " .");
//	ostringstream maplog;
//	for(map<string, vector<int> >::const_iterator mIt = m_laneIndexMap.begin();mIt != m_laneIndexMap.end();++mIt)
//	{
//		maplog << mIt->first <<'[';
//		for(vector<int>::const_iterator vIt = mIt->second.begin();vIt != mIt->second.end();++vIt)
//			maplog<<*vIt<<", ";
//		maplog << "] ";
//	}
//	WRITE_MESSAGE("Map content " + maplog.str());
    );
}

void MSSwarmTrafficLightLogic::resetPheromone() {
    //input
    for (MSLaneId_PheromoneMap::iterator laneIterator = pheromoneInputLanes.begin();
            laneIterator != pheromoneInputLanes.end(); laneIterator++) {
        std::string laneId = laneIterator->first;
        pheromoneInputLanes[laneId] = 0;
    }
    //output
    for (MSLaneId_PheromoneMap::iterator laneIterator = pheromoneOutputLanes.begin();
            laneIterator != pheromoneOutputLanes.end(); laneIterator++) {
        std::string laneId = laneIterator->first;
        pheromoneOutputLanes[laneId] = 0;
    }
}

int MSSwarmTrafficLightLogic::decideNextPhase() {

    DBG(
        MsgHandler::getMessageInstance()->inform("\n" + time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic decideNextPhase()"); std::ostringstream dnp; dnp << (MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::decideNextPhase:: " << "tlsid=" << getID() << " getCurrentPhaseDef().getState()=" << getCurrentPhaseDef().getState() << " is commit?" << getCurrentPhaseDef().isCommit(); MsgHandler::getMessageInstance()->inform(dnp.str());)
    // if we're congested, it should be wise to reset and recalculate the pheromone levels after X steps

    if (getCurrentPhaseDef().isTarget()) {
        targetLanes = getCurrentPhaseDef().getTargetLaneSet();
    }

    if (getCurrentPolicy()->getName().compare("Congestion") == 0 && getCurrentPhaseDef().isCommit()) {
        congestion_steps += 1;	//STEPS2TIME(getCurrentPhaseDef().duration);
        DBG(
            WRITE_MESSAGE("\n" + time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic decideNextPhase()"); std: ostringstream dnp; dnp << (MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::decideNextPhase:: " << "tlsid=" << getID() << " congestion_steps=" << congestion_steps; WRITE_MESSAGE(dnp.str());)
        if (congestion_steps >= getMaxCongestionDuration()) {
            resetPheromone();
            congestion_steps = 0;
            mustChange = true;
            if (getReinforcementMode() != 0) {
                skipEta = true;
            }
            DBG(
                WRITE_MESSAGE("\n" + time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic decideNextPhase()"); std::ostringstream dnp; dnp << (MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::decideNextPhase:: " << "tlsid=" << getID() << " max congestion reached, congestion_steps=" << congestion_steps; WRITE_MESSAGE(dnp.str());)
        }
    }

    //Update pheromone levels
    updatePheromoneLevels();

    /* Since we changed the behaviour of computeReturnTime() in order to update pheromone levels every step
     * it is now mandatory to check if the duration of a transient phase is elapsed or not*/
    if (getCurrentPhaseDef().isTransient() && getCurrentPhaseElapsed() < getCurrentPhaseDef().duration) {
        return getCurrentPhaseIndex();
    }

    //Decide the current policy according to pheromone levels. this should be done only at the end of a chain, before selecting the new one
    if (getCurrentPhaseDef().isCommit()) {
        //Update learning and forgetting thresholds
        updateSensitivities();
        decidePolicy();
        gotTargetLane = false;
    }

//    double phero =0;
//	if(getCurrentPhaseDef().isDecisional())
//	{
//    for(LaneIdVector::const_iterator it = targetLanes.begin(); it != targetLanes.end(); ++it)
//    {
//      string name = (*it);
//      phero +=pheromoneInputLanes[name];
//    }
//    phero /= targetLanes.size() == 0 ? 1 : targetLanes.size();
//    if(getCurrentPhaseElapsed() >= getCurrentPhaseDef().minDuration)
//      if(abs(phero-pheroBegin) <= 2)
//        return getCurrentPhaseIndex() + 1;
//	}
    DBG(
        std::ostringstream str; str << "tlsID=" << getID() << " currentPolicyname=" + getCurrentPolicy()->getName(); WRITE_MESSAGE(str.str());)

    //Execute current policy. congestion "policy" must maintain the commit phase, and that must be an all-red one
    return getCurrentPolicy()->decideNextPhase(getCurrentPhaseElapsed(), &getCurrentPhaseDef(), getCurrentPhaseIndex(),
            getPhaseIndexWithMaxCTS(), isThresholdPassed(), isPushButtonPressed(), countVehicles(getCurrentPhaseDef()));
//	int newStep =getCurrentPolicy()->decideNextPhase(getCurrentPhaseElapsed(), &getCurrentPhaseDef(), getCurrentPhaseIndex(),
//	          getPhaseIndexWithMaxCTS(), isThresholdPassed(), isPushButtonPressed(), countVehicles(getCurrentPhaseDef()));
//	if(newStep != myStep)
//	  pheroBegin = phero;
//	return newStep;
}

void MSSwarmTrafficLightLogic::updatePheromoneLevels() {
    //Updating input lanes pheromone: all input lanes without distinction
    //BETA_NO, GAMMA_NO
    updatePheromoneLevels(pheromoneInputLanes, "PheroIn", getBetaNo(), getGammaNo());

    //BETA_SP, GAMMA_SP
    //Updating output lanes pheromone: only input lanes currently having green light. Pheromone for non green lanes is "freezed"
//    if (getCurrentPhaseDef().isDecisional()) {
    updatePheromoneLevels(pheromoneOutputLanes, "PheroOut", getBetaSp(), getGammaSp());
//    }
}

void MSSwarmTrafficLightLogic::updatePheromoneLevels(MSLaneId_PheromoneMap& pheroMap, std::string logString,
        const double beta, const double gamma) {
    //	ANALYSIS_DBG(
    DBG(
        std::ostringstream _str; _str << logString << " Lanes " << pheroMap.size() << " TL " << getID() << " ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::updatePheromoneLevels:: " + _str.str());)

    for (MSLaneId_PheromoneMap::iterator laneIterator = pheroMap.begin(); laneIterator != pheroMap.end();
            ++laneIterator) {
        std::string laneId = laneIterator->first;
        double oldPhero = laneIterator->second;
        double maxSpeed = getSensors()->getMaxSpeed(laneId);
        double meanVehiclesSpeed = getSensors()->meanVehiclesSpeed(laneId);
        bool updatePheromone = (meanVehiclesSpeed > -1);
        //		double pheroAdd = getSensors()->countVehicles(laneId);

        //derivative
        double derivative = 0;
        //If i need to use the derivative for the lane
        if (m_meanSpeedHistory.find(laneId) != m_meanSpeedHistory.end()) {
            //Update the derivative
            if (updatePheromone) {
                double currentDerivative = 0;
                m_losCounter = 0;
                if (m_meanSpeedHistory[laneId]->size() > 0) {
                    //Calculate the current derivative mean with the old speed points
                    for (int i = 0; i < m_meanSpeedHistory[laneId]->size(); ++i)
                        if (i == 0) {
                            currentDerivative += fabs(meanVehiclesSpeed - m_meanSpeedHistory[laneId]->at(i));
                        } else {
                            currentDerivative += fabs(m_meanSpeedHistory[laneId]->at(i - 1) - m_meanSpeedHistory[laneId]->at(i));
                        }
                    currentDerivative /= m_meanSpeedHistory[laneId]->size(); //Non weighted mean
                }
                m_meanSpeedHistory[laneId]->push_front(meanVehiclesSpeed);
                //Check if the current value of the derivative is above the set alpha
                if (currentDerivative >= m_derivativeAlpha) {
                    m_derivativeHistory[laneId]->push_front(currentDerivative);
                }
                if (m_derivativeHistory[laneId]->size() > 0) {
                    //Calculate the mean derivative with the old derivative
                    for (int i = 0; i < m_derivativeHistory[laneId]->size(); ++i) {
                        derivative += m_derivativeHistory[laneId]->at(i);
                    }
                    derivative /= m_derivativeHistory[laneId]->size();
                }
            } else {
                //Reset the values if no information is received after a timeout
                ++m_losCounter;
                if (m_losCounter >= m_losMaxLimit) {
                    m_derivativeHistory[laneId]->clear();
                    m_meanSpeedHistory[laneId]->clear();
                    m_meanSpeedHistory[laneId]->push_front(maxSpeed);
                }
            }
        }
        double pheroAdd = MAX2((maxSpeed - meanVehiclesSpeed) * 10 / maxSpeed, 0.0);
//		Use the derivative only if it has a value
        if (derivative > 0)
//		Correct the pheromone value by dividing it for the derivative.
        {
            pheroAdd /= MAX2(derivative, m_derivativeAlpha);
        }
//    pheroAdd /= max(derivative, 1.0);
        ANALYSIS_DBG(
        if (updatePheromone) {
        std::ostringstream oss;
        oss << time2string(MSNet::getInstance()->getCurrentTimeStep()) << " l " << laneId;
            oss << " der " << derivative << " phero " << pheroAdd << " maxS " << maxSpeed << " meanS " << meanVehiclesSpeed;
            WRITE_MESSAGE(oss.str())
        }
        )

        // Evaporation + current contribute
        double phero = beta * oldPhero + gamma * pheroAdd * updatePheromone;
        ANALYSIS_DBG(
        if (phero > 10) {
        std::ostringstream i_str;
        i_str << "MSSwarmTrafficLightLogic::updatePheromoneLevels " << logString << " > 10. Value: " << phero;
        WRITE_MESSAGE(i_str.str())
        });

        phero = MIN2(MAX2(phero, 0.0), getPheroMaxVal());
        pheroMap[laneId] = phero;
        ANALYSIS_DBG(
            //		DBG(
            std::ostringstream i_str;
            //					i_str << " oldPheroIn " << oldPheroIn
            //						<< " inMeanVehiclesSpeed " << meanVehiclesSpeed
            //						<< " pheroInAdd " << pheroAdd * updatePheromoneIn
            //						<< " pheroInEvaporated " << oldPheroIn-oldPheroIn*getBetaNo()
            //						<< " pheroInDeposited " << getGammaNo() * pheroAdd * updatePheromoneIn
            //						<<" newPheroIn "<<pheromoneInputLanes[laneId]
            //						<< " inLane "<< laneId<<" ID "<< getID() <<" .";
            i_str << " op " << oldPhero << " ms " << meanVehiclesSpeed << " p " << pheroAdd * updatePheromone <<
            " pe " << oldPhero - oldPhero * beta << " pd " << gamma * pheroAdd * updatePheromone << " np " <<
        pheroMap[laneId] << " l " << laneId << " ID " << getID() << " c " << getSensors()->countVehicles(laneId) << " s " << getLaneLightState(laneId) << " ."; if (m_pheroLevelLog[laneId] != i_str.str()) {
        m_pheroLevelLog[laneId] = i_str.str();
            WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::updatePheromoneLevels:: " + logString + i_str.str());
        })

        DBG(
            std::ostringstream str; str << time2string(MSNet::getInstance()->getCurrentTimeStep()) << " MSSwarmTrafficLightLogic::countSensors:: lane " << laneId << " passedVeh " << getCountSensors()->getPassedVeh(laneId, false); WRITE_MESSAGE(str.str());)

//		int vehicles = getSensors()->countVehicles(laneId);
//		double pheroIn = getBetaNo() * oldPheroIn + // Evaporation
//		getGammaNo() * vehicles;
//		DBG(
//		std::ostringstream i_str;
//		i_str << " vehicles " << getSensors()->countVehicles(laneId)<<" pheromoneInputLanes "<<pheromoneInputLanes[laneId] << " lane "<< laneId<<" ID "<<  getID() <<" .";
//		MsgHandler::getMessageInstance()->inform(time2string(MSNet::getInstance()->getCurrentTimeStep()) +" MSSwarmTrafficLightLogic::updatePheromoneLevels:: PheroIn"+i_str.str());
//		)
//
//      pheroIn = MIN2(MAX2(pheroIn, 0.0), getPheroMaxVal());
//		pheromoneInputLanes[laneId] = pheroIn;
    }
}
void MSSwarmTrafficLightLogic::updateSensitivities() {
    double elapsedTime = STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() - lastThetaSensitivityUpdate);
    lastThetaSensitivityUpdate = MSNet::getInstance()->getCurrentTimeStep();

    MSSOTLPolicy* currentPolicy = getCurrentPolicy();
    std::vector<MSSOTLPolicy*> policies = getPolicies();

    //reset of the sensitivity thresholds in case of 0 pheromone on the input lanes
    if (getPheromoneForInputLanes() == 0) {
        for (int i = 0; i < (int)policies.size(); i++) {
            policies[i]->setThetaSensitivity(getThetaInit());
//			ANALYSIS_DBG(
            DBG(
                std::ostringstream phero_str; phero_str << "Policy " << policies[i]->getName() << " sensitivity reset to " << policies[i]->getThetaSensitivity() << " due to evaporated input pheromone."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::updateSensitivities::" + phero_str.str());)
        }
        return;
    }

    double eta = -1.;
    // If skipEta it means that we've had Congestion for too much time. Forcing forgetting.
    if (!skipEta || currentPolicy->getName().compare("Congestion") != 0) {
        switch (getReinforcementMode()) {
            case 0:
                if (elapsedTime == STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep())) {
                    return;	//we don't want to reinforce the policy selected at the beginning of the simulation since it's time-based
                }
                eta = elapsedTime;
                break;
            case 1:
                eta = calculateEtaDiff();
                break;
            case 2:
                eta = calculateEtaRatio();
                break;
        }
    }
    for (int i = 0; i < (int)policies.size(); i++) {
        MSSOTLPolicy* policy = policies[i];
        double newSensitivity;
        if (eta < 0) {	//bad performance
            if (policy == currentPolicy) { // punish the current policy
                newSensitivity = policy->getThetaSensitivity() + getForgettingCox() * (-eta);
            } else
                // reward the other ones
            {
                newSensitivity = policy->getThetaSensitivity() - getLearningCox() * (-eta);
            }
        } else {	//good performance
            if (policy == currentPolicy) {	//reward the current policy
                newSensitivity = policy->getThetaSensitivity() - getLearningCox() * eta;
            } else
                //	punish the other ones
            {
                newSensitivity = policy->getThetaSensitivity() + getForgettingCox() * eta;
            }
        }
//			ANALYSIS_DBG(
        DBG(
        std::ostringstream lf; std::ostringstream phero_str; if (getReinforcementMode() == 0) {
        if (policy == currentPolicy) {
                lf << " ,LearningCox " << getLearningCox() << " ,LCox*Time " << getLearningCox() * elapsedTime;
            } else {
                lf << " ,ForgettingCox " << getForgettingCox() << " ,FCox*Time " << getForgettingCox() * elapsedTime;
            }

            phero_str << " policy " << policy->getName() << " newSensitivity " << newSensitivity << " ,pol.Sensitivity " << policy->getThetaSensitivity() << " ,elapsedTime " << elapsedTime << lf.str() << " NEWERSensitivity= " << max(min(newSensitivity, getThetaMax()), getThetaMin()) << " ID " << getID() << " .";
        } else {
            if (policy == currentPolicy && eta > 0) {
                lf << " ,LearningCox " << getLearningCox() << " ,LCox*Eta " << getLearningCox() * eta;
            } else if (policy == currentPolicy && eta < 0) {
                lf << " ,ForgettingCox " << getForgettingCox() << " ,FCox*Eta " << getForgettingCox() * eta;
            } else if (eta > 0) {
                lf << " ,ForgettingCox " << getForgettingCox() << " ,FCox*Eta " << getForgettingCox() * eta;
            } else if (eta < 0) {
                lf << " ,LearningCox " << getLearningCox() << " ,LCox*Eta " << getLearningCox() * eta;
            }
            phero_str << " policy " << policy->getName() << " newSensitivity " << newSensitivity << " ,pol.Sensitivity " << policy->getThetaSensitivity() << " ,eta " << eta << " ,carsIn " << carsIn << " ,inTarget " << inTarget << " ,notTarget " << notTarget << " ,carsOut " << carsOut << lf.str() << " NEWERSensitivity= " << max(min(newSensitivity, getThetaMax()), getThetaMin()) << " ID " << getID() << " .";
        }
        WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::updateSensitivities::" + phero_str.str());)

        newSensitivity = MAX2(MIN2(newSensitivity, getThetaMax()), getThetaMin());
        policy->setThetaSensitivity(newSensitivity);
    }
}

double MSSwarmTrafficLightLogic::getPheromoneForInputLanes() {
    if (pheromoneInputLanes.size() == 0) {
        return 0;
    }
    double pheroIn = 0;
    for (MSLaneId_PheromoneMap::const_iterator iterator = pheromoneInputLanes.begin();
            iterator != pheromoneInputLanes.end(); iterator++) {
        std::string laneId = iterator->first;
        pheroIn += iterator->second;
        DBG(
            std::ostringstream phero_str; phero_str << " lane " << iterator->first << " pheromoneIN  " << iterator->second << " id " << getID() << " ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::getPheromoneForInputLanes::" + phero_str.str());)
    }

    DBG(
        std::ostringstream o_str; o_str << " TOTpheromoneIN  " << pheroIn << " return  " << pheroIn / pheromoneInputLanes.size() << getID() << " ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::getPheromoneForInputLanes::" + o_str.str());)
    return pheroIn / pheromoneInputLanes.size();
}

double MSSwarmTrafficLightLogic::getPheromoneForOutputLanes() {
    if (pheromoneOutputLanes.size() == 0) {
        return 0;
    }
    double pheroOut = 0;
    for (MSLaneId_PheromoneMap::const_iterator iterator = pheromoneOutputLanes.begin();
            iterator != pheromoneOutputLanes.end(); iterator++) {
        DBG(
            std::ostringstream phero_str; phero_str << " lane " << iterator->first << " pheromoneOUT  " << iterator->second << " id " << getID() << " ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::getPheromoneForOutputLanes::" + phero_str.str());)
        pheroOut += iterator->second;
    }
    DBG(
        std::ostringstream o_str; o_str << " TOTpheromoneOUT  " << pheroOut << " return  " << pheroOut / pheromoneOutputLanes.size() << " id " << getID() << " ."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::getPheromoneForOutputLanes::" + o_str.str());)
    return pheroOut / pheromoneOutputLanes.size();
}

double MSSwarmTrafficLightLogic::getDispersionForInputLanes(double average_phero_in) {
    if (pheromoneInputLanes.size() == 0) {
        return 0;
    }
    double sum = 0;
    for (MSLaneId_PheromoneMap::const_iterator iterator = pheromoneInputLanes.begin();
            iterator != pheromoneInputLanes.end(); iterator++) {
        std::string laneId = iterator->first;
        sum += pow(iterator->second - average_phero_in, 2);
    }

    double result = sqrt(sum / pheromoneInputLanes.size()) * getScaleFactorDispersionIn();
    DBG(
        ostringstream so_str; so_str << " dispersionIn " << result; WRITE_MESSAGE("MSSwarmTrafficLightLogic::getDispersionForInputLanes::" + so_str.str());)
    return result;
}

double MSSwarmTrafficLightLogic::getDispersionForOutputLanes(double average_phero_out) {
    if (pheromoneOutputLanes.size() == 0) {
        return 0;
    }
    double sum = 0;
    for (MSLaneId_PheromoneMap::const_iterator iterator = pheromoneOutputLanes.begin();
            iterator != pheromoneOutputLanes.end(); iterator++) {
        sum += pow(iterator->second - average_phero_out, 2);
    }

    double result = sqrt(sum / pheromoneOutputLanes.size()) * getScaleFactorDispersionOut();
    DBG(
        ostringstream so_str; so_str << " dispersionOut " << result; WRITE_MESSAGE("MSSwarmTrafficLightLogic::getDispersionForOutputLanes::" + so_str.str());)
    return result;
}
double MSSwarmTrafficLightLogic::getDistanceOfMaxPheroForInputLanes() {
    if (pheromoneInputLanes.size() == 0) {
        return 0;
    }
    double max_phero_val_current = 0;
    double max_phero_val_old = 0;
    double temp_avg_other_lanes = 0;
    std::string laneId_max;
    int counter = 0;
    for (MSLaneId_PheromoneMap::const_iterator iterator = pheromoneInputLanes.begin();
            iterator != pheromoneInputLanes.end(); iterator++) {
        std::string laneId = iterator->first;
        double lanePhero = iterator->second;
        if (counter == 0) {
            max_phero_val_current = lanePhero;
            counter++;
            continue;
        }
        if (lanePhero > max_phero_val_current) {
            max_phero_val_old = max_phero_val_current;
            max_phero_val_current = lanePhero;
            temp_avg_other_lanes = (temp_avg_other_lanes * (counter - 1) + max_phero_val_old) / counter;
        } else {
            temp_avg_other_lanes = (temp_avg_other_lanes * (counter - 1) + lanePhero) / counter;
        }

        counter++;
    }

    double result = max_phero_val_current - temp_avg_other_lanes;
    DBG(
        ostringstream so_str; so_str << " currentMaxPhero " << max_phero_val_current << " lane " << laneId_max << " avgOtherLanes " << temp_avg_other_lanes << " distance " << result; WRITE_MESSAGE("MSSwarmTrafficLightLogic::getDistanceOfMaxPheroForInputLanes::" + so_str.str());)
    return result;
}

double MSSwarmTrafficLightLogic::getDistanceOfMaxPheroForOutputLanes() {
    if (pheromoneOutputLanes.size() == 0) {
        return 0;
    }
    double max_phero_val_current = 0;
    double max_phero_val_old = 0;
    double temp_avg_other_lanes = 0;
    std::string laneId_max;
    int counter = 0;
    for (MSLaneId_PheromoneMap::const_iterator iterator = pheromoneOutputLanes.begin();
            iterator != pheromoneOutputLanes.end(); iterator++) {
        std::string laneId = iterator->first;
        double lanePhero = iterator->second;
        if (counter == 0) {
            max_phero_val_current = lanePhero;
            counter++;
            continue;
        }
        if (lanePhero > max_phero_val_current) {
            max_phero_val_old = max_phero_val_current;
            max_phero_val_current = lanePhero;
            temp_avg_other_lanes = (temp_avg_other_lanes * (counter - 1) + max_phero_val_old) / counter;
        } else {
            temp_avg_other_lanes = (temp_avg_other_lanes * (counter - 1) + lanePhero) / counter;
        }

        counter++;
    }

    double result = max_phero_val_current - temp_avg_other_lanes;
    DBG(
        ostringstream so_str; so_str << " currentMaxPhero " << max_phero_val_current << " lane " << laneId_max << " avgOtherLanes " << temp_avg_other_lanes << " distance " << result; WRITE_MESSAGE("MSSwarmTrafficLightLogic::getDistanceOfMaxPheroForOutputLanes::" + so_str.str());)
    return result;
}
void MSSwarmTrafficLightLogic::decidePolicy() {
//	MSSOTLPolicy* currentPolicy = getCurrentPolicy();
    // Decide if it is the case to check for another plan
//	double sampled = (double) RandHelper::rand(RAND_MAX);
    double sampled = RandHelper::rand();
    double changeProb = getChangePlanProbability();
//	changeProb = changeProb * RAND_MAX;

    if (sampled <= changeProb || mustChange) { // Check for another plan

        double pheroIn = getPheromoneForInputLanes();
        double pheroOut = getPheromoneForOutputLanes();
        //double dispersionIn = getDispersionForInputLanes(pheroIn);
        //double dispersionOut = getDispersionForOutputLanes(pheroOut);
        double distancePheroIn = getDistanceOfMaxPheroForInputLanes();
        double distancePheroOut = getDistanceOfMaxPheroForOutputLanes();
        MSSOTLPolicy* oldPolicy = getCurrentPolicy();
        choosePolicy(pheroIn, pheroOut, distancePheroIn, distancePheroOut);
        MSSOTLPolicy* newPolicy = getCurrentPolicy();

        if (newPolicy != oldPolicy) {
            ANALYSIS_DBG(
                SUMOTime step = MSNet::getInstance()->getCurrentTimeStep(); std::ostringstream phero_str; phero_str << " (pheroIn= " << pheroIn << " ,pheroOut= " << pheroOut << " )"; WRITE_MESSAGE("TL " + getID() + " time " + time2string(step) + " Policy: " + newPolicy->getName() + phero_str.str() + " OldPolicy: " + oldPolicy->getName() + " id " + getID() + " .");)
            if (oldPolicy->getName().compare("Congestion") == 0) {
                congestion_steps = 0;
            }
        } else { //debug purpose only
            ANALYSIS_DBG(
                std::ostringstream phero_str; phero_str << " (pheroIn= " << pheroIn << " ,pheroOut= " << pheroOut << " )"; SUMOTime step = MSNet::getInstance()->getCurrentTimeStep(); WRITE_MESSAGE("TL " + getID() + " time " + time2string(step) + " Policy: Nochanges" + phero_str.str() + " OldPolicy: " + oldPolicy->getName() + " id " + getID() + " .");)
        }

        mustChange = false;
        skipEta = false;
    }
}

double MSSwarmTrafficLightLogic::calculatePhi(int factor) {
    if (factor == 0) {
        return 1;
    }
    if (factor == 1) {
        return 0.2;
    } else {
        return 1 - (1 / ((double) factor));
    }
}

double MSSwarmTrafficLightLogic::calculateEtaDiff() {

    MSLane* currentLane = nullptr;
    int count = 0, minIn = 0, minOut = 0, toSub, tmp;
    bool inInit = true, outInit = true;
    double eta, normalized, diff, phi, delta;
    LaneIdVector toReset;

    carsIn = 0;
    carsOut = 0;
    inTarget = 0;
    notTarget = 0;

    MSSOTLE2Sensors* sensors = (MSSOTLE2Sensors*) getCountSensors();

    // Search the incoming lane to get the count of the vehicles passed. [IN]
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector = myLanes.begin();
            laneVector != myLanes.end(); laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane = laneVector->begin(); lane != laneVector->end();
                lane++) {
            currentLane = (*lane);

            // Map to avoid check the lane for every possible direction
            if (laneCheck[currentLane] == false) {
                // Get the vehicles passed from this lane.
                count = sensors->getPassedVeh(currentLane->getID(), false);

                DBG(
                    std::ostringstream cars_str; cars_str << "Lane " << currentLane->getID() << ": vehicles entered - " << count; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + cars_str.str());)

                // Increment the global count of the cars passed through the tl
                carsIn += count;
                // Set to true to skip similar lane since there's just one sensor
                laneCheck[currentLane] = true;
            }
        }
    }

    // Search the outgoing lane to get the count of the vehicles passed. [OUT]
    // We use the links to get the respective lane id.
    for (MSTrafficLightLogic::LinkVectorVector::const_iterator linkVector = myLinks.begin();
            linkVector != myLinks.end(); linkVector++) {
        for (MSTrafficLightLogic::LinkVector::const_iterator link = linkVector->begin(); link != linkVector->end();
                link++) {
            currentLane = (*link)->getLane();

            // Map to avoid check the lane for every possible direction
            if (laneCheck[currentLane] == false) {
                // Get the vehicles passed from this lane.
                count = sensors->getPassedVeh(currentLane->getID(), true);

                DBG(
                    std::ostringstream cars_str; cars_str << "Lane " << currentLane->getID() << ": vehicles gone out- " << count; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + cars_str.str());)

                // Increment the global count of the cars passed through the tl
                carsOut += count;

                // Since there's no output target lanes we check here the minimum number of
                // cars passed though the tl. This ahs to be done to all the output lanes since cars can go
                // in any direction from a target lane. If a direction isn't reachable the sensor count will be 0.
                // This is done to update the sensorCount value in order to don't make it grow too much.
                if (count != 0) {
                    toReset.push_back(currentLane->getID());
                    if (outInit) {
                        minOut = count;
                        outInit = false;
                    } else if (count <= minOut) {
                        minOut = count;
                    }
                }
                // Set to true to skip similar lane since there's just one sensor
                laneCheck[currentLane] = true;
            }
        }
    }
    // Reset the map to check again all the lane on the next commit.
    resetLaneCheck();

    // We retrieve the minimum number of cars passed from the target lanes.
    for (LaneIdVector::const_iterator laneId = targetLanes.begin(); laneId < targetLanes.end(); laneId++) {
        std::string lane = (*laneId);
        tmp = sensors->getPassedVeh(lane, false);
        inTarget += tmp;
        if (inInit && tmp != 0) {
            minIn = tmp;
            inInit = false;
        }
        if (tmp < minIn && tmp != 0) {
            minIn = tmp;
        }
        if (tmp != 0) {
            toReset.push_back(lane);
        }
        DBG(
            std::ostringstream cars_str; cars_str << "Lane " << lane << " passed: " << tmp; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + cars_str.str());)
    }

    // The cars not on a target lane counted as in.
    notTarget = carsIn - inTarget;

    // Calculate the min beetween the min number of cars entered the tl (minIn) and the
    // ones that have exit the tl (minOut)
    toSub = std::min(minIn, minOut);

    // Subtract the value to all the sensor on the target lanes.
    while (!toReset.empty()) {
        std::string laneId = toReset.back();
        toReset.pop_back();
        sensors->subtractPassedVeh(laneId, toSub);
    }

    //Normalized to 1
    diff = inTarget - carsOut;
    normalized = diff / inTarget;

    // Analize difference to return an appropriate eta to reinforce/forget the policies.

    DBG(
        std::ostringstream final_str; final_str << "Total cars in lanes: " << carsIn << " Total cars out: " << carsOut << " Difference: " << diff << " Pure eta: " << normalized; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + final_str.str());)
    DBG(
        std::ostringstream eta_str; eta_str << "IN:" << inTarget << " OUT:" << carsOut << " R:" << notTarget; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + eta_str.str());)
    DBG(
        std::ostringstream eta_str; eta_str << "Min found:" << toSub << " MinIn:" << minIn << " MinOut:" << minOut; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + eta_str.str());)

    // IN > OUT
    if (inTarget > carsOut) {
        if (carsOut == 0) {
            // We're in Congestion but not for long so we don't do nothing. When we reach max steps for
            // Congestion the evaluation of eta is skipped and we force a forget of the policy
            if (getCurrentPolicy()->getName().compare("Congestion") == 0) {
                eta = 0;
            }
            // vehicles aren't going out and we've additional vehicle on a red lane. We set
            // eta to -1 to forget
            else {
                eta = -1;
            }
        } else {
            // Forget - Amplify to R
            phi = calculatePhi(notTarget);
            eta = (-normalized * (1 / phi));
            if (eta < -1.0) {
                eta = -1.0;
            }
        }
    }

    // IN = OUT
    else if (inTarget == carsOut) {
        // Can't say nothing
        if (inTarget == 0) {
            eta = 0;
        }

        // Reinforce - Attenuate to R
        // Normalized = 0 --> use delta = 1-1/IN
        else {
            delta = calculatePhi(inTarget);
            phi = calculatePhi(notTarget);
            eta = delta * phi;
            if (eta > 1.0) {
                eta = 1.0;
            }
        }
    }

    // IN < OUT
    else {
        // Can't say nothing
        if (inTarget == 0) {
            eta = 0;
        }

        // Reinforce - Attenuate to R
        else {
            phi = calculatePhi(notTarget);
            diff = inTarget - carsOut;
            normalized = diff / carsOut;
            eta = normalized * phi;
            if (eta > 1.0) {
                eta = 1.0;
            }
        }
    }

    DBG(
        std::ostringstream eta_str; eta_str << "Eta Normalized: " << eta; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + eta_str.str());)
    return eta;
}

double MSSwarmTrafficLightLogic::calculateEtaRatio() {
    MSLane* currentLane = nullptr;
    int count = 0, minIn = 0, minOut = 0, toSub, tmp;
    bool inInit = true, outInit = true;
    double eta, ratio, phi, normalized, delta;
    LaneIdVector toReset;

    carsIn = 0;
    carsOut = 0;
    inTarget = 0;
    notTarget = 0;

    MSSOTLE2Sensors* sensors = (MSSOTLE2Sensors*) getCountSensors();

    // Search the incoming lane to get the count of the vehicles passed. [IN]
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector = myLanes.begin();
            laneVector != myLanes.end(); laneVector++) {
        for (MSTrafficLightLogic::LaneVector::const_iterator lane = laneVector->begin(); lane != laneVector->end();
                lane++) {
            currentLane = (*lane);

            // Map to avoid check the lane for every possible direction
            if (laneCheck[currentLane] == false) {
                // Get the vehicles passed from this lane.
                count = sensors->getPassedVeh(currentLane->getID(), false);

                DBG(
                    std::ostringstream cars_str; cars_str << "Lane " << currentLane->getID() << ": vehicles entered - " << count; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + cars_str.str());)

                // Increment the global count of the cars passed through the tl
                carsIn += count;
                // Set to true to skip similar lane since there's just one sensor
                laneCheck[currentLane] = true;
            }
        }
    }

    // Search the outgoing lane to get the count of the vehicles passed. [OUT]
    // We use the links to get the respective lane id.
    for (MSTrafficLightLogic::LinkVectorVector::const_iterator linkVector = myLinks.begin();
            linkVector != myLinks.end(); linkVector++) {
        for (MSTrafficLightLogic::LinkVector::const_iterator link = linkVector->begin(); link != linkVector->end();
                link++) {
            currentLane = (*link)->getLane();

            // Map to avoid check the lane for every possible direction
            if (laneCheck[currentLane] == false) {
                // Get the vehicles passed from this lane.
                count = sensors->getPassedVeh(currentLane->getID(), true);

                DBG(
                    std::ostringstream cars_str; cars_str << "Lane " << currentLane->getID() << ": vehicles gone out- " << count; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + cars_str.str());)

                // Increment the global count of the cars passed through the tl
                carsOut += count;

                // Since there's no output target lanes we check here the minimum number of
                // cars passed though the tl. This has to be done to all the output lanes since cars can go
                // in any direction from a target lane. If a direction isn't reachable the sensor count will be 0.
                // This is done to update the sensorCount value in order to don't make it grow too much.
                if (count != 0) {
                    toReset.push_back(currentLane->getID());
                    if (outInit) {
                        minOut = count;
                        outInit = false;
                    } else if (count <= minOut) {
                        minOut = count;
                    }
                }

                // Set to true to skip similar lane since there's just one sensor
                laneCheck[currentLane] = true;
            }
        }
    }
    // Reset the map to check again all the lane on the next commit.
    resetLaneCheck();

    // We retrieve the minimum number of cars passed from the target lanes.
    for (LaneIdVector::const_iterator laneId = targetLanes.begin(); laneId < targetLanes.end(); laneId++) {
        std::string lane = (*laneId);
        tmp = sensors->getPassedVeh(lane, false);
        inTarget += tmp;
        if (inInit && tmp != 0) {
            minIn = tmp;
            inInit = false;
        }
        if (tmp < minIn && tmp != 0) {
            minIn = tmp;
        }
        if (tmp != 0) {
            toReset.push_back(lane);
        }
        DBG(
            std::ostringstream cars_str; cars_str << "Lane " << lane << " passed: " << tmp; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + cars_str.str());)
    }

    // The cars not on a target lane counted as in.
    notTarget = carsIn - inTarget;

    // Calculate the min beetween the min number of cars entered the tl (minIn) and the
    // ones that have exit the tl (minOut)
    toSub = std::min(minIn, minOut);

    // Subtract the value to all the sensor on the target lanes.
    while (!toReset.empty()) {
        std::string laneId = toReset.back();
        toReset.pop_back();
        sensors->subtractPassedVeh(laneId, toSub);
    }

    //Normalized to 1
    if (carsOut != 0) {
        ratio = ((double) inTarget) / carsOut;
        normalized = ratio / (inTarget + carsOut);
    } else {
        ratio = std::numeric_limits<double>::infinity();
        normalized = std::numeric_limits<double>::infinity();
    }

    DBG(
        std::ostringstream final_str; final_str << "Total cars in lanes: " << carsIn << " Total cars out: " << carsOut << " Ratio: " << ratio << " Pure eta: " << normalized; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + final_str.str());)
    DBG(
        std::ostringstream eta_str; eta_str << "IN:" << inTarget << ". OUT:" << carsOut << " R:" << notTarget; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + eta_str.str());)
    DBG(
        std::ostringstream eta_str; eta_str << "Min found:" << toSub << ". MinIn:" << minIn << " MinOut:" << minOut; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + eta_str.str());)
    // Analize ratio to return an appropriate eta to reinforce/forget the policies.

    // IN > OUT
    if (inTarget > carsOut) {
        if (carsOut == 0) {
            // we're in Congestion but not for long so we don't do nothing. When we reach max steps for
            // Congestion the evaluation of eta is skipped and we force a forget of the policy
            if (getCurrentPolicy()->getName().compare("Congestion") == 0) {
                eta = 0;
            }
            // vehicles aren't going out and we've additional vehicle on a red lane. We set
            // eta to -1 to forget
            else {
                eta = -1;
            }
        } else {
            // Forget according to the ratio. Amplify due to the cars in the red lanes
            phi = calculatePhi(notTarget);
            eta = (-(normalized) * (1 / phi));
            if (eta < -1.0) {
                eta = -1.0;
            }
        }
    }
    // IN = OUT
    else if (inTarget == carsOut) {
        // We can't say nothing.
        if (inTarget == 0) {
            eta = 0;
        }
        // Reinforce - Attenuate to R
        // same number of vehicles that are getting IN is getting OUT
        // Normalized = 1/TOT ---> change to delta = 1-1/IN
        else {
            delta = calculatePhi(inTarget);
            phi = calculatePhi(notTarget);
            eta = delta * phi;
            if (eta > 1.0) {
                eta = 1.0;
            }
        }
    }
    // IN < OUT
    else {
        // We can't say nothing.
        if (inTarget == 0) {
            eta = 0;
        }

        // There was a queue and now cars are getting over it
        // There're vehicles on the red lanes (R)
        // We reinforce and attenuate according to R
        else {
            phi = calculatePhi(notTarget);
            eta = (normalized) * phi;
            if (eta > 1.0) {
                eta = 1.0;
            }
        }
    }

    DBG(
        std::ostringstream eta_str; eta_str << "Eta Normalized: " << eta << "."; WRITE_MESSAGE(time2string(MSNet::getInstance()->getCurrentTimeStep()) + " MSSwarmTrafficLightLogic::calculateEta::" + eta_str.str());)
    return eta;

}

void MSSwarmTrafficLightLogic::resetLaneCheck() {

    MSLane* currentLane = nullptr;

    // reset both the input and the output lanes.
    for (MSTrafficLightLogic::LaneVectorVector::const_iterator laneVector = myLanes.begin();
            laneVector != myLanes.end(); laneVector++) {

        for (MSTrafficLightLogic::LaneVector::const_iterator lane = laneVector->begin(); lane != laneVector->end();
                lane++) {
            currentLane = (*lane);
            laneCheck[currentLane] = false;
        }
    }

    for (MSTrafficLightLogic::LinkVectorVector::const_iterator linkVector = myLinks.begin();
            linkVector != myLinks.end(); linkVector++) {
        for (MSTrafficLightLogic::LinkVector::const_iterator link = linkVector->begin(); link != linkVector->end();
                link++) {
            currentLane = (*link)->getLane();
            laneCheck[currentLane] = false;
        }
    }
}

void MSSwarmTrafficLightLogic::choosePolicy(double phero_in, double phero_out, double dispersion_in,
        double dispersion_out) {
    if (m_useVehicleTypesWeights) {
        for (std::vector<MSSOTLPolicy*>::iterator it = getPolicies().begin(); it != getPolicies().end(); ++it) {
            if (it.operator * ()->getName() == "Phase") {
                activate(*it);
                return;
            }
        }
    }
    std::vector<double> thetaStimuli;
    double thetaSum = 0.0;
    // Compute stimulus for each policy
    for (int i = 0; i < (int)getPolicies().size(); i++) {
        double stimulus = getPolicies()[i]->computeDesirability(phero_in, phero_out, dispersion_in, dispersion_out);
        double thetaStimulus = pow(stimulus, 2) / (pow(stimulus, 2) + pow(getPolicies()[i]->getThetaSensitivity(), 2));

        thetaStimuli.push_back(thetaStimulus);
        thetaSum += thetaStimulus;

//		ANALYSIS_DBG(
        DBG(
            ostringstream so_str; so_str << " policy " << getPolicies()[i]->getName() << " stimulus " << stimulus << " pow(stimulus,2) " << pow(stimulus, 2) << " pow(Threshold,2) " << pow(getPolicies()[i]->getThetaSensitivity(), 2) << " thetaStimulus " << thetaStimulus << " thetaSum " << thetaSum << " TL " << getID(); WRITE_MESSAGE("MSSwarmTrafficLightLogic::choosePolicy::" + so_str.str());)

    }

    // Compute a random value between 0 and the sum of the thetaSum
//	double r = RandHelper::rand(RAND_MAX);
//	r = r / RAND_MAX * thetaSum;
    double r = RandHelper::rand((double)thetaSum);

    double partialSum = 0;
    for (int i = 0; i < (int)getPolicies().size(); i++) {
        partialSum += thetaStimuli[i];

//		ANALYSIS_DBG(
        DBG(
            ostringstream aao_str; aao_str << " policy " << getPolicies()[i]->getName() << " partialSum " << partialSum << " thetaStimuls " << thetaStimuli[i] << " r " << r << " TL " << getID(); WRITE_MESSAGE("MSSwarmTrafficLightLogic::choosePolicy::" + aao_str.str());)

        if (partialSum >= r) {
            activate(getPolicies()[i]);
            break;
        }
    }
}

void MSSwarmTrafficLightLogic::choosePolicy(double phero_in, double phero_out) {
    choosePolicy(phero_in, phero_out, 0, 0);
}

//never called...
bool MSSwarmTrafficLightLogic::canRelease() {
    DBG(
        std::ostringstream phero_str; phero_str << "getCurrentPhaseElapsed()=" << time2string(getCurrentPhaseElapsed()) << " isThresholdPassed()=" << isThresholdPassed() << " currentPhase=" << (&getCurrentPhaseDef())->getState() << " countVehicles()=" << countVehicles(getCurrentPhaseDef()); WRITE_MESSAGE("MSSwamTrafficLightLogic::canRelease(): " + phero_str.str());)
    return getCurrentPolicy()->canRelease(getCurrentPhaseElapsed(), isThresholdPassed(), isPushButtonPressed(), &getCurrentPhaseDef(),
                                          countVehicles(getCurrentPhaseDef()));
}

std::string MSSwarmTrafficLightLogic::getLaneLightState(const std::string& laneId) {
    std::string laneState = "";
    if (m_laneIndexMap.find(laneId) != m_laneIndexMap.end()) {
        std::string state = getCurrentPhaseDef().getState();
        for (std::vector<int>::const_iterator it = m_laneIndexMap[laneId].begin(); it != m_laneIndexMap[laneId].end(); ++it) {
            laneState += state[*it];
        }
    }
    return laneState;
}
