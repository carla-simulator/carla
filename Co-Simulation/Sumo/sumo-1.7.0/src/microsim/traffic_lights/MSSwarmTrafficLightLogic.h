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
/// @file    MSSwarmTrafficLightLogic.h
/// @author  Gianfilippo Slager
/// @author  Federico Caselli
/// @date    Mar 2010
///
// The class for Swarm-based logics
/****************************************************************************/
#pragma once
#include <config.h>

//#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include "MSSOTLHiLevelTrafficLightLogic.h"
#include "MSSOTLPhasePolicy.h"
#include "MSSOTLPlatoonPolicy.h"
#include "MSSOTLMarchingPolicy.h"
#include "MSSOTLCongestionPolicy.h"
#include "MSSOTLPolicy3DStimulus.h"
#include "MSSOTLPolicy5DFamilyStimulus.h"

template<class T>
class CircularBuffer {
public:
    CircularBuffer(int size) :
        m_size(size), m_currentIndex(0), m_firstTime(true) {
        m_buffer = new T[m_size];
    }

    virtual ~CircularBuffer() {
        delete[] m_buffer;
    }

    bool addValue(const T newValue, T& replacedValue) {
        bool result = !m_firstTime;
        if (result) {
            replacedValue = m_buffer[m_currentIndex];
        }
        insert(newValue);
        return result;
    }

    void push_front(const T value) {
        insert(value);
    }

    T at(const int index) const {
        int idx = (m_currentIndex - 1 - index + m_size) % m_size;
        return m_buffer[idx];
    }

    T front() const {
        return at(0);
    }

    T back() const {
        return at(size() - 1);
    }

    int size() const {
        if (m_firstTime) {
            return m_currentIndex;
        }
        return m_size;
    }

    void clear() {
        m_currentIndex = 0;
        m_firstTime = true;
    }

private:
    T* m_buffer;
    int m_size;
    int m_currentIndex;
    bool m_firstTime;

    inline void insert(const T& value) {
        m_buffer[m_currentIndex++] = value;
        if (m_currentIndex == m_size) {
            m_currentIndex = 0;
            m_firstTime = false;
        }
    }
};

class MSSwarmTrafficLightLogic: public MSSOTLHiLevelTrafficLightLogic {
public:
    //****************************************************
    /**
     * @brief Constructor without sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameters Parameters defined for the tll
     */
    MSSwarmTrafficLightLogic(MSTLLogicControl& tlcontrol, const std::string& id,
                             const std::string& programID, const Phases& phases, int step,
                             SUMOTime delay,
                             const std::map<std::string, std::string>& parameters);

    ~MSSwarmTrafficLightLogic();

    /**
     * @brief Initialises the tls with sensors on incoming and outgoing lanes
     * Sensors are built in the simulation according to the type of sensor specified in the simulation parameter
     * @param[in] nb The detector builder
     * @exception ProcessError If something fails on initialisation
     */
    void init(NLDetectorBuilder& nb);

    SUMOTime getMaxCongestionDuration() {
        return StringUtils::toInt(getParameter("MAX_CONGESTION_DUR", "120"));
    }

    double getPheroMaxVal() {
        return StringUtils::toDouble(getParameter("PHERO_MAXVAL", "10"));
    }

    double getBetaNo() {
        return StringUtils::toDouble(getParameter("BETA_NO", "0.99"));
    }

    double getGammaNo() {
        return StringUtils::toDouble(getParameter("GAMMA_NO", "1.0"));
    }

    double getBetaSp() {
        return StringUtils::toDouble(getParameter("BETA_SP", "0.99"));
    }

    double getGammaSp() {
        return StringUtils::toDouble(getParameter("GAMMA_SP", "1.0"));
    }

    double getChangePlanProbability() {
        return StringUtils::toDouble(getParameter("CHANGE_PLAN_PROBABILITY", "0.003"));
    }

    double getThetaMax() {
        return StringUtils::toDouble(getParameter("THETA_MAX", "0.8"));
    }

    double getThetaMin() {
        return StringUtils::toDouble(getParameter("THETA_MIN", "0.2"));
    }

    double getThetaInit() {
        return StringUtils::toDouble(getParameter("THETA_INIT", "0.5"));
    }

    double getLearningCox() {
        return StringUtils::toDouble(getParameter("LEARNING_COX", "0.0005"));
    }

    double getForgettingCox() {
        return StringUtils::toDouble(getParameter("FORGETTING_COX", "0.0005"));
    }

    double getScaleFactorDispersionIn() {
        return scaleFactorDispersionIn;
    }

    double getScaleFactorDispersionOut() {
        return scaleFactorDispersionOut;
    }

    /** @brief Returns the type of the logic as a string
     * @return The type of the logic
     */
    const std::string getLogicType() const {
        return "swarmBasedTrafficLogic";
    }
    /// @}

protected:
    /**
     * @brief This pheronome is an indicator of congestion on input lanes.\n
     * Its levels refer to the average speed of vehicles passing the input lane:
     * the lower the speed the higher the pheromone.\n
     * These levels are updated on every input lane, independently on lights state.
     */
    MSLaneId_PheromoneMap pheromoneInputLanes;

    /**
     * \brief This pheromone is an indicator of congestion on output lanes.\n
     * Its levels refer to the average speed of vehicles passing the output lane:
     * the lower the speed the higher the pheromone.\n
     * These levels are updated on every output lane, independently on lights state.
     */
    MSLaneId_PheromoneMap pheromoneOutputLanes;

    /**
     * This member keeps track of the last thresholds update, s.t.
     * updates can be correctly performed even on time-variable interations.
     * @see MSSwarmTrafficLightLogic::updateSensitivities()
     */
    SUMOTime lastThetaSensitivityUpdate;

    /*
     * This member has to contain the switching logic for SOTL policies
     */

    int decideNextPhase();

    bool canRelease();

    /*
     * Computes how much time will pass after decideNextPhase will be executed again
     */
    virtual SUMOTime computeReturnTime() {

        return DELTA_T;

    }

    /**
     * @brief Resets pheromone levels
     */
    void resetPheromone();

    /*
     * @return The average pheromone level regarding congestion on input lanes
     */
    double getPheromoneForInputLanes();

    /*
     * @return The average pheromone level regarding congestion on output lanes
     */
    double getPheromoneForOutputLanes();

    /*
     * @return The dispersion level regarding congestion on input lanes
     */
    double getDispersionForInputLanes(double average_phero_in);

    /*
     * @return The dispersion level regarding congestion on output lanes
     */
    double getDispersionForOutputLanes(double average_phero_out);

    /*
     * @return The difference between the current max phero value and the average phero of the other lanes
     */
    double getDistanceOfMaxPheroForInputLanes();

    /*
     * @return The difference between the current max phero value and the average phero of the other lanes
     */
    double getDistanceOfMaxPheroForOutputLanes();
    /**
     * @brief Update pheromone levels
     * Pheromone on input lanes is costantly updated
     * Pheromone follows a discrete-time dynamic law "pheromone(k+1) = beta*pheromone(k) + gamma * sensed_val(k)"
     */
    void updatePheromoneLevels();

    /**
     * @brief Utility method to avoid code duplication
     */
    void updatePheromoneLevels(MSLaneId_PheromoneMap&, std::string, const double, const double);

    /**
     * After a policy has been chosen, for every iteration thresholds has to be updated.
     * Thresholds reinforcement lowers the theta_sensitivity for the current policy and raises the ones for currently unused policies.
     * Thresholds belongs to the interval [THETA_MIN THETA_MAX]
     */
    void updateSensitivities();

    /**
     * @brief Decide the current policy according to pheromone levels
     * The decision reflects on currentPolicy value
     */
    void decidePolicy();

    /**
     * \brief Method that should calculate the valor of phi a coefficient to amplify/attenuate eta based on a factor.
     * The factor depends on the situation when the function is called; should be the number of cars in the target lanes
     * or the number of cars in the lanes with a red tl.
     * @param[in] factor - the value to consider to compute this coefficient.
     */
    double calculatePhi(int factor);

    /**
     * \brief Method that should calculate the valor of eta a coefficient to evaluate the current
     * policy's work. This eta is based on the difference between the number of vehicles that enters a tl
     * and the ones that exit it. It consider vehicles on a lane with a tl set to red as well to determinate
     * policy work.
     */
    double calculateEtaDiff();

    double calculateEtaRatio();

    /*
     * \brief Method to reset the map that stores if a lane is already been checked during the
     * evaluation of eta.
     */
    void resetLaneCheck();
    void choosePolicy(double phero_in, double phero_out, double dispersion_in, double dispersion_out);
    void choosePolicy(double phero_in, double phero_out);

    std::string getPoliciesParam() {
        return getParameter("POLICIES", "Platoon;Phase;Marching;Congestion");
    }

    /*
     * Reinforcement modes:
     * 0-> elapsed time
     * 1-> diff
     * 2-> ratio
     */
    int getReinforcementMode() {
        return StringUtils::toInt(getParameter("REIMODE", "0"));
    }

    void initScaleFactorDispersionIn(int lanes_in) {
        std::vector<double> phero_values;

        for (int i = 0; i < lanes_in / 2; i++) {
            phero_values.push_back(getPheroMaxVal());
        }
        for (int i = lanes_in / 2; i < lanes_in; i++) {
            phero_values.push_back(0.0);
        }

        double sum_avg_tmp = 0;

        for (int i = 0; i < (int)phero_values.size(); i++) {
            sum_avg_tmp += phero_values[i];
        }

        double mean = sum_avg_tmp / phero_values.size();

        double sum_dev_tmp = 0;
        for (int i = 0; i < (int)phero_values.size(); i++) {
            sum_dev_tmp += pow(phero_values[i] - mean, 2);
        }

        double deviation = sqrt(sum_dev_tmp / phero_values.size());

        scaleFactorDispersionIn = getPheroMaxVal() / deviation;
    }

    void initScaleFactorDispersionOut(int lanes_out) {
        std::vector<double> phero_values;

        for (int i = 0; i < lanes_out / 2; i++) {
            phero_values.push_back(getPheroMaxVal());
        }
        for (int i = lanes_out / 2; i < lanes_out; i++) {
            phero_values.push_back(0.0);
        }

        double sum_avg_tmp = 0;
        for (int i = 0; i < (int)phero_values.size(); i++) {
            sum_avg_tmp += phero_values[i];
        }
        double mean = sum_avg_tmp / phero_values.size();

        double sum_dev_tmp = 0;

        for (int i = 0; i < (int)phero_values.size(); i++) {
            sum_dev_tmp += pow(phero_values[i] - mean, 2);
        }

        double deviation = sqrt(sum_dev_tmp / phero_values.size());

        scaleFactorDispersionOut = getPheroMaxVal() / deviation;
    }

    /**
     * @brief Check if a lane is allowed to be added to the maps pheromoneInputLanes and pheromoneOutputLanes
     * Control in this function if the lane is a walking area, a crossing, or if only pedestrian are allowed.
     * Return true if the lane has to be added, false otherwise.
     */
    bool allowLine(MSLane*);

    bool logData;
    std::ofstream swarmLogFile;
    /**
     * \brief When true, indicates that the current policy MUST be changed.\n
     * It's used to force the exit from the congestion policy
     */
    bool mustChange;
    SUMOTime congestion_steps;

    /**
     * \brief Map to check if a lane was already controlled during the elaboration of eta.
     */
    LaneCheckMap laneCheck;
    /**
     * \brief A copy of the target lanes of this phase.
     */
    LaneIdVector targetLanes;
    /**
     * \brief When true indicates that we can skip the evaluation of eta since we've
     * a congestion policy that is lasting too much.
     */
    bool skipEta;
    /**
     * \brief When true indicates that we've already acquired the target lanes for this
     * particular phase.
     */
    bool gotTargetLane;

    int carsIn;
    int carsOut;
    int inTarget;
    int notTarget;
    /**
     * \factors to scale pheromoneDispersion in range [0, 10]
     */
    double scaleFactorDispersionIn;
    double scaleFactorDispersionOut;

//	For every lane its index. Esed to get the current lane state for the lane
    std::map<std::string, std::vector<int> > m_laneIndexMap;
    std::string getLaneLightState(const std::string& laneId);
//	store the last message logged. if equal do not log it again
    std::map<std::string, std::string> m_pheroLevelLog;

    //derivative
    std::map<std::string, CircularBuffer<double>* > m_meanSpeedHistory;
    std::map<std::string, CircularBuffer<double>* > m_derivativeHistory;
    double m_derivativeAlpha;
    int m_losCounter;//los: loss of signal
    int m_losMaxLimit;
    bool m_useVehicleTypesWeights;

//	double pheroBegin;
};
