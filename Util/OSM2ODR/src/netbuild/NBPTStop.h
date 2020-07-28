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
/// @file    NBPTStop.h
/// @author  Gregor Laemmel
/// @date    Tue, 20 Mar 2017
///
// The representation of a single pt stop
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/geom/Position.h>
#include "utils/common/SUMOVehicleClass.h"
#include "NBPTPlatform.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class NBEdgeCont;
class NBEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class NBPTStop
* @brief The representation of a single pt stop
*/
class NBPTStop {

public:
    /**@brief Constructor
    * @param[in] id The id of the pt stop
    * @param[in] position The position of the pt stop
    * @param[in] edgeId The edge id of the pt stop
    * @param[in] length The length of the pt stop
    */
    NBPTStop(std::string ptStopId, Position position, std::string edgeId, std::string origEdgeId, double length, std::string name,
             SVCPermissions svcPermissions, double parkingLength = 0);
    std::string getID() const;

    const std::string getEdgeId() const;
    const std::string getOrigEdgeId() const;
    const std::string getName() const;
    const Position& getPosition() const;
    SVCPermissions getPermissions() const;
    void write(OutputDevice& device);
    void reshiftPosition(const double offsetX, const double offsetY);

    const std::vector<NBPTPlatform>& getPlatformCands();
    bool getIsMultipleStopPositions() const;
    void setIsMultipleStopPositions(bool multipleStopPositions);
    double getLength() const;
    bool setEdgeId(std::string edgeId, const NBEdgeCont& ec);
    void registerAdditionalEdge(std::string wayId, std::string edgeId);
    void addPlatformCand(NBPTPlatform platform);
    bool findLaneAndComputeBusStopExtent(const NBEdgeCont& ec);

    bool findLaneAndComputeBusStopExtent(const NBEdge* edge);

    void setMyPTStopId(std::string id);
    void addAccess(std::string laneID, double offset, double length);

    /// @brief remove all access definitions
    void clearAccess();

    /// @brief register line that services this stop (for displaying)
    void addLine(const std::string& line);

    void setBidiStop(NBPTStop* bidiStop) {
        myBidiStop = bidiStop;
    }

    NBPTStop* getBidiStop() const {
        return myBidiStop;
    }

    bool isLoose() const {
        return myIsLoose;
    }

    /// @brief mirror coordinates along the x-axis
    void mirrorX();

private:
    void computeExtent(double center, double d);

private:
    std::string myPTStopId;
    Position myPosition;
    std::string myEdgeId;
    std::map<std::string, std::string> myAdditionalEdgeCandidates;
public:
    const std::map<std::string, std::string>& getMyAdditionalEdgeCandidates() const;
private:
    std::string myOrigEdgeId;
public:
    void setMyOrigEdgeId(const std::string& myOrigEdgeId);
private:
    double myPTStopLength;
public:
    void setMyPTStopLength(double myPTStopLength);
private:
    const std::string myName;
    const double myParkingLength;
    std::string myLaneId;
    const SVCPermissions myPermissions;

    double myStartPos;
    double myEndPos;

    /// @brief laneId, lanePos, accessLength
    std::vector<std::tuple<std::string, double, double>> myAccesses;

    /// @brief list of public transport lines (for displaying)
    std::vector<std::string> myLines;

    NBPTStop* myBidiStop;

    /// @brief whether the stop was not part of the road network and must be mapped
    bool myIsLoose;

private:
    /// @brief Invalidated assignment operator.
    NBPTStop& operator=(const NBPTStop&);


    std::vector<NBPTPlatform> myPlatformCands;
    bool myIsMultipleStopPositions;
};

