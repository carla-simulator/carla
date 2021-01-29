/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    TrajectoriesHandler.h
/// @author  Michael Behrisch
/// @date    14.03.2014
///
// An XML-Handler for amitran and netstate trajectories
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utility>
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TrajectoriesHandler
 * @brief An XML-Handler for amitran and netstate trajectories
 *
 * This SUMOSAXHandler parses vehicles and their speeds.
 */
class TrajectoriesHandler : public SUMOSAXHandler {
public:
    static const int INVALID_VALUE = -999999;

public:
    /** @brief Constructor
     *
     * @param[in] file The file that will be processed
     */
    TrajectoriesHandler(const bool computeA, const bool computeAForward, const bool accelZeroCorrection,
                        const SUMOEmissionClass defaultClass,
                        const double defaultSlope, std::ostream* stdOut, OutputDevice* xmlOut);


    /// @brief Destructor
    ~TrajectoriesHandler();

    const PollutantsInterface::Emissions computeEmissions(const std::string id,
            const SUMOEmissionClass c, double& v,
            double& a, double& s);

    bool writeEmissions(std::ostream& o, const std::string id,
                        const SUMOEmissionClass c,
                        double t, double& v,
                        double& a, double& s);

    bool writeXMLEmissions(const std::string id,
                           const SUMOEmissionClass c,
                           SUMOTime t, double& v,
                           double a = INVALID_VALUE, double s = INVALID_VALUE);

    void writeSums(std::ostream& o, const std::string id);

    void writeNormedSums(std::ostream& o, const std::string id, const double factor);


protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called when an opening-tag occurs
     *
     * Processes vehicle and motionState elements.
     *
     * @param[in] element The enum of the currently opened element
     * @param[in] attrs Attributes of the currently opened element
     * @exception ProcessError If an error within the parsed file occurs
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);
    //@}


private:
    const bool myComputeA;
    const bool myComputeAForward;
    const bool myAccelZeroCorrection;
    const SUMOEmissionClass myDefaultClass;
    const double myDefaultSlope;
    std::ostream* myStdOut;
    OutputDevice* myXMLOut;
    std::map<std::string, double> myLastV;
    std::map<std::string, double> myLastSlope;
    SUMOTime myCurrentTime;
    double myStepSize;
    std::map<std::string, PollutantsInterface::Emissions> mySums;
    std::map<std::string, SUMOEmissionClass> myEmissionClassByType;
    std::map<std::string, SUMOEmissionClass> myEmissionClassByVehicle;


private:
    /// @brief invalidated copy constructor
    TrajectoriesHandler(const TrajectoriesHandler& s);

    /// @brief invalidated assignment operator
    TrajectoriesHandler& operator=(const TrajectoriesHandler& s);


};
