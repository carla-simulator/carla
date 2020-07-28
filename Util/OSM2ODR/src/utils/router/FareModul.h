/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    FareModul.h
/// @author  Ricardo Euler
/// @date    Thu, 17 August 2018
///
// Fare Modul for calculating prices during intermodal routing
/****************************************************************************/
#pragma once
#include <config.h>

#include <cassert>
#include <string>
#include <vector>
#include "EffortCalculator.h"
#include "FareToken.h"
#include "FareZones.h"


// ===========================================================================
// class definitions
// ===========================================================================
class ZoneCounter {
public:

    explicit ZoneCounter(unsigned int ct) :
        myCount(ct) {

    }

    inline void addZone(int zoneNumber) {
        zoneNumber = getOverlayZone(zoneNumber);
        if (zoneNumber == 0) {
            return;
        }
        long long int repNum = fareZoneToRep[zoneNumber];
        //assert power of 2
        if (bitcount(repNum) == 0) {
            return;
        }
        myCount = myCount | repNum;
    }


    int numZones() const {
        return bitcount(myCount);
    }


private:
    inline int bitcount(long long int intVal) const {
        int count = 0;
        long long int counter = intVal;

        while (counter != 0) {
            counter = counter & (counter - 1);
            ++count;
        }
        return count;
    }

private:
    long long int myCount;


};



/**
 * A fare state collects all the information that is necessary to compute the price. Is used as an edge label
 * in IntermodalRouter
 */
struct FareState {
    friend class FareModul;

public:

    /** default constructor for unlabeled edges**/
    explicit  FareState():
        myFareToken(FareToken::None),
        myCounter(std::numeric_limits<int>::max()),
        myTravelledDistance(std::numeric_limits<double>::max()),
        myVisistedStops(std::numeric_limits<int>::max()),
        myPriceDiff(0) {
    };

    /**
     *
     * @param token
     */
    explicit FareState(FareToken token):
        myFareToken(token),
        myCounter(0),
        myTravelledDistance(0),
        myVisistedStops(0),
        myPriceDiff(0) {}

    /** Destructor **/
    ~FareState() = default;

    /**
     * returns true if fare state is set and not on default
     * @return if state is set
     */
    bool isValid() const {
        return !(myFareToken == FareToken::None);
    }

private:

    /** fare token **/
    FareToken myFareToken;
    /** zone counter **/
    ZoneCounter myCounter;
    /** travelled distance in km**/
    double myTravelledDistance;
    /**num of visited stops**/
    int myVisistedStops;
    /** price diff to previous edge **/
    double myPriceDiff;

};



struct Prices {



    /** Prices for zones **/
    std::vector<double> zonePrices = std::vector<double> {1.9, 3.4, 4.9, 6.2, 7.7, 9.2};
    double  halle = 2.3;
    double leipzig = 2.7;
    double t1 = 1.5;
    double t2 = 1.6;
    double t3 = 1.6;
    double shortTrip = 1.6;
    double shortTripLeipzig = 1.9;
    double shortTripHalle = 1.7;
    double maxPrice = 10.6;
};


/**
 * The fare modul responsible for calculating prices
 */
class FareModul : public EffortCalculator {
public:

    /** Constructor ***/
    FareModul() :
        myFareStates()
    {};

    /**Implementation of EffortCalculator **/
    void init(const std::vector<std::string>& edges) override {
        myEdges = edges;
        myFareStates.resize(edges.size());
    }

    void addStop(const int stopEdge, const Parameterised& params) override {
        myStopFareZone[stopEdge] = StringUtils::toInt(params.getParameter("fareZone"));
        myStopFareToken[stopEdge] = FareUtil::stringToToken(params.getParameter("fareToken"));
        myStopStartToken[stopEdge] = FareUtil::stringToToken(params.getParameter("startToken"));
    }

    /**Implementation of EffortCalculator **/
    double getEffort(const int numericalID) const override {
        double  effort = 0;
        FareState const& state =  myFareStates.at(numericalID);
        if (state.isValid()) {
            effort = state.myPriceDiff;
        } else {
            effort = std::numeric_limits<double>::max();
        }
        return effort;
    }

    /** Implementation of EffortCalculator **/
    void update(const int edge, const int prev, const double length) override {

        std::string const& edgeType = myEdges[edge];

        //get propagated fare state
        FareState& state  = myFareStates.at(prev);

        double oldPr;
        if (state.myFareToken == FareToken::START) {
            oldPr = 0;
        } else {
            oldPr = computePrice(state);
        }
        //treat  public transport edges
        if (edgeType.c_str()[0] != '!') {
            updateFareStatePublic(state, edge, length);
        } else if (edgeType ==  "!stop") {
            updateFareStateStop(state, edge);
        } else if (edgeType == "!ped") {
            updateFareStatePedestrian(state, edge);
        } else if (edgeType == "!access") {
            updateFareStateAccess(state, edge, prev);
        } else {
            updateFareState(state, edge);
        }
        FareState& stateAtE = myFareStates[edge];
        double newPr = computePrice(stateAtE);
        stateAtE.myPriceDiff = newPr - oldPr;

        assert(stateAtE.myPriceDiff  >= 0);

    }

    /** Implementation of EffortCalculator
     *  _IntermodalEdge should be an Connector Edge  **/
    void setInitialState(const int edge) override {
//    assert( edge->getLine() == "!connector");

        myFareStates[edge] = FareState(FareToken::START);

    }


private:
    /** List of all fare states **/
    std::vector<FareState> myFareStates;

    /** List of all edge line attributes **/
    std::vector<std::string> myEdges;

    /** the fare zone this stop is a part of **/
    std::map<int, int> myStopFareZone;

    /** the faretoken that can be collected at this station **/
    std::map<int, FareToken> myStopFareToken;

    /** the faretoken that is used when a trip is started at this station **/
    std::map<int, FareToken> myStopStartToken;

    /** List of the prices **/
    Prices prices;

    double computePrice(FareState const& fareState) const {
        switch (fareState.myFareToken) {
            case FareToken ::H:
                return prices.halle;
            case FareToken ::L:
                return prices.leipzig;
            case FareToken ::T1:
                return prices.t1;
            case FareToken ::T2:
                return prices.t2;
            case FareToken ::T3:
                return prices.t3;
            case FareToken::U:
                return prices.zonePrices[0];
            case FareToken ::Z:
                return prices.zonePrices[fareState.myCounter.numZones() - 1];
            case FareToken ::M:
                return prices.maxPrice;
            case FareToken ::K:
                return prices.shortTrip;
            case FareToken ::KL:
            case FareToken ::KLZ:
            case FareToken ::KLU:
                return prices.shortTripLeipzig;
            case FareToken ::KH:
            case FareToken ::KHU:
            case FareToken ::KHZ:
                return prices.shortTripHalle;
            case FareToken::Free:
                return 1.4;
            case FareToken ::START:
                return 0;
            case FareToken::ZU:
            case FareToken::None:
                assert(false);

        }
        return std::numeric_limits<double>::max();
    }



    std::string output(const int edge) const override {

        FareState const&   my = myFareStates[edge];
        std::stringstream msg;
        /*
                msg << "Final fare state at edge of type: " << myEdges[edge] << std::endl;
                msg << "Faretoken" << FareUtil::tokenToString(my.myFareToken) << std::endl;
                msg << "Price:" << computePrice(my) << std::endl;
                msg << "Zones " << my.myCounter.numZones() << std::endl;
                msg << "Stations: " << my.myVisistedStops << std::endl;
                msg << "Distance:" << my.myTravelledDistance << std::endl;
        */
        msg << FareUtil::tokenToTicket(my.myFareToken) << " ";
        if (my.myFareToken == FareToken::Z) {
            msg << my.myCounter.numZones() << " ";
            if (my.myCounter.numZones() == 1) {
                msg << "Zone";
            } else {
                msg << "Zonen";
            }

        } else if (my.myFareToken == FareToken::U) {
            msg << my.myCounter.numZones() << "1 Zone";

        }
        msg << ":" << computePrice(my);
        return msg.str();
    }

    void updateFareStateStop(FareState const& currentFareState, const int stopEdge) {

        FareToken  collectedToken = myStopFareToken[stopEdge];

        //if station has no fare information, just propagate
        if (collectedToken  == FareToken::None) {
            std::cout << "Progagating fare state for stop w/o a price!" << std::endl;
            return;
        }

        FareToken const& token = currentFareState.myFareToken;

        FareState& stateAtE = myFareStates[stopEdge];

        stateAtE = currentFareState;

        stateAtE.myCounter.addZone(myStopFareZone[stopEdge]);

        stateAtE.myVisistedStops++;

        switch (token) {
            case FareToken ::Free:
                stateAtE.myFareToken = myStopStartToken[stopEdge];
                break;
            case FareToken::M :
                break;

            case FareToken::Z :
                if (stateAtE.myCounter.numZones() > 6) {
                    stateAtE.myFareToken = FareToken::M;
                }
                break;

            case FareToken::T1 :
            case FareToken::T2 :
            case FareToken::T3 :
                if (collectedToken == FareToken::Z) {
                    stateAtE.myFareToken = stateAtE.myTravelledDistance <= 4000 ? FareToken::K : FareToken::Z;
                }
                break;
            case FareToken::U :
                if (collectedToken == FareToken::H) {
                    stateAtE.myFareToken = FareToken::H;
                }
                if (collectedToken == FareToken::L) {
                    stateAtE.myFareToken = FareToken::L;
                }
                if (collectedToken == FareToken::Z) {
                    stateAtE.myFareToken = FareToken::Z;
                }
                break;
            case FareToken::H:
            case FareToken::L:
                if (collectedToken == FareToken::Z) {
                    stateAtE.myFareToken = FareToken::Z;
                }
                break;
            case FareToken::KH:
                if (stateAtE.myVisistedStops <= 4) {
                    if (collectedToken == FareToken::U) {
                        stateAtE.myFareToken = FareToken::KHU;
                    }
                    if (collectedToken == FareToken::Z) {
                        stateAtE.myFareToken = FareToken::KHZ;
                    }
                } else {
                    if (collectedToken == FareToken::H) {
                        stateAtE.myFareToken = FareToken ::H;
                    }
                    if (collectedToken == FareToken::Z) {
                        stateAtE.myFareToken = FareToken ::Z;
                    }
                    if (collectedToken == FareToken::U) {
                        stateAtE.myFareToken = FareToken ::U;
                    }
                }
                break;
            case FareToken::KL:
                if (stateAtE.myVisistedStops <= 4) {
                    if (collectedToken == FareToken::U) {
                        stateAtE.myFareToken = FareToken::KLU;
                    }
                    if (collectedToken == FareToken::Z) {
                        stateAtE.myFareToken = FareToken::KLZ;
                    }
                } else {
                    if (collectedToken == FareToken::L) {
                        stateAtE.myFareToken = FareToken ::L;
                    }
                    if (collectedToken == FareToken::Z) {
                        stateAtE.myFareToken = FareToken ::Z;
                    }
                    if (collectedToken == FareToken::U) {
                        stateAtE.myFareToken = FareToken ::U;
                    }
                }
                break;
            case FareToken::K:
                if (stateAtE.myTravelledDistance > 4000) {
                    if (collectedToken == FareToken::U) {
                        stateAtE.myFareToken = FareToken ::U;
                    }
                    if (collectedToken == FareToken::Z) {
                        stateAtE.myFareToken = FareToken ::Z;
                    }
                }
                break;
            case FareToken::KHU :
            case FareToken::KLU :
                if (stateAtE.myVisistedStops > 4) {
                    if (collectedToken == FareToken::U) {
                        stateAtE.myFareToken = FareToken::U;
                    }
                }
                break;

            case FareToken::KLZ:
            case FareToken::KHZ:
                if (stateAtE.myVisistedStops > 4) {
                    if (collectedToken == FareToken::Z) {
                        stateAtE.myFareToken = FareToken::Z;
                    }
                }
                break;
            case FareToken::ZU :
                assert(false);
                if (collectedToken == FareToken::U) {
                    stateAtE.myFareToken = FareToken::U;
                } else {
                    stateAtE.myFareToken = FareToken::Z;
                }

                break;
            default:
                std::cout << "Reached invalid position in fareToken selection!" << std::endl;
                assert(false);
                break;
        }
    }

    void updateFareStatePedestrian(FareState const& currentFareState, const int pedestrianEdge) {

        //only propagates the fare state
        FareState& stateAtE = myFareStates[pedestrianEdge];

        stateAtE = currentFareState;

        if (currentFareState.myFareToken == FareToken::START) {
            stateAtE.myFareToken = FareToken::Free;
        }

    }


    void updateFareStatePublic(FareState const& currentFareState, const int publicTransportEdge, const double length) {


        if (currentFareState.myFareToken == FareToken::None) {
            return;
        }

        FareState& stateAtE = myFareStates[publicTransportEdge];

        stateAtE = currentFareState;
        stateAtE.myTravelledDistance += length;
    }

    void updateFareState(FareState const& currentFareState, const int intermodalEdge) {

        if (currentFareState.myFareToken == FareToken::None) {
            return;
        }

        FareState& stateAtE = myFareStates[intermodalEdge];

        stateAtE = currentFareState;

        if (currentFareState.myFareToken == FareToken::START) {
            stateAtE.myFareToken = FareToken::Free;
        }

    }

    void updateFareStateAccess(FareState const& currentFareState, const int accessEdge, const int prev) {

        FareToken const& token = currentFareState.myFareToken;

        FareState& stateAtE = myFareStates[accessEdge];

        stateAtE = currentFareState;

        if (currentFareState.myFareToken == FareToken::START) {
            stateAtE.myFareToken = FareToken::Free;
        }

        if (myEdges[prev] == "!ped") {
            switch (token) {

                case FareToken::Free ://we have not yet taken public transport
                    break;
                case  FareToken::K :
                    if (currentFareState.myCounter.numZones() == 0) {
                        stateAtE.myFareToken = FareToken::U;
                    } else {
                        stateAtE.myFareToken = FareToken::Z;
                    }
                    break;
                case  FareToken::KH :
                    stateAtE.myFareToken = FareToken::H;
                    break;
                case  FareToken::KL :
                    stateAtE.myFareToken = FareToken::L;
                    break;
                case  FareToken::KLU :
                    stateAtE.myFareToken = FareToken::L;
                    break;
                case  FareToken::KHU:
                    stateAtE.myFareToken = FareToken::H;
                    break;
                case  FareToken::KLZ :
                    stateAtE.myFareToken = FareToken::Z;
                    break;
                case  FareToken::KHZ:
                    stateAtE.myFareToken = FareToken::Z;
                    break;
                default:
                    return;
            }
        }

    }
};


