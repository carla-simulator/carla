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
/// @file    MEInductLoop.h
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// An induction loop for mesoscopic simulation
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <deque>
#include <map>
#include <functional>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/output/MSMeanData_Net.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/Named.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MESegment;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MEInductLoop
 * @brief An induction loop for mesoscopic simulation
 */
class MEInductLoop : public MSDetectorFileOutput {
public:
    MEInductLoop(const std::string& id,
                 MESegment* s,
                 double positionInMeters,
                 const std::string& vTypes);


    ~MEInductLoop();

    /**
     * @name Inherited MSDetectorFileOutput methods.
     *
     * @see MSDetectorFileOutput
     */
    //@{
    /** @brief Opens the XML-output using "detector" as root element
     *
     * @param[in] dev The output device to write the root into
     * @see MSDetectorFileOutput::writeXMLDetectorProlog
     */
    void writeXMLDetectorProlog(OutputDevice& dev) const {
        dev.writeXMLHeader("detector", "det_e1meso_file.xsd");
    }


    /**
     * Get the XML-formatted output of all the get*-methods except
     * getTimeSinceLastDetection.
     *
     * @param lastNTimesteps take data out of the interval
     * (now-lastNTimesteps, now].
     *
     * @return XML-formatted output of all the get*-methods except
     * getTimeSinceLastDetection.
     *
     * @see MSDetector2File
     */
    void writeXMLOutput(OutputDevice& dev,
                        SUMOTime startTime, SUMOTime stopTime);
    //@}

protected:
    /// @brief mesoscopic edge segment the loop lies on
    MESegment* const mySegment;

    /// @brief position from the start of the edge / lane
    const double myPosition;

    /// @brief data collector for the loop
    MSMeanData_Net::MSLaneMeanDataValues myMeanData;

private:

    /// Hidden default constructor.
    MEInductLoop();

    /// Hidden copy constructor.
    MEInductLoop(const MEInductLoop&);

    /// Hidden assignment operator.
    MEInductLoop& operator=(const MEInductLoop&);
};
