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
/// @file    MSJunction.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Wed, 12 Dez 2001
///
// The base class for an intersection
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <map>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <utils/common/Named.h>
#include <utils/common/Parameterised.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;
class MSLink;
class MSLane;
class MSEdge;
class MSJunctionLogic;

typedef std::vector<MSEdge*> MSEdgeVector;
typedef std::vector<const MSEdge*> ConstMSEdgeVector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSJunction
 * @brief The base class for an intersection
 */
class MSJunction : public Named, public Parameterised {
public:
    /** @brief Constructor
     * @param[in] id The id of the junction
     * @param[in] position The position of the junction
     * @param[in] shape The shape of the junction
     */
    MSJunction(const std::string& id,
               SumoXMLNodeType type,
               const Position& position,
               const PositionVector& shape,
               const std::string& name);


    /// @brief Destructor.
    virtual ~MSJunction();


    /** performs some initialisation after the loading
        (e.g., link map computation) */
    virtual void postloadInit();


    /** returns the junction's position */
    const Position& getPosition() const;


    /** @brief Returns this junction's shape
     * @return The shape of this junction
     */
    const PositionVector& getShape() const {
        return myShape;
    }

    /// @brief return the junction name
    const std::string& getName() const {
        return myName;
    }

    virtual const std::vector<MSLink*>& getFoeLinks(const MSLink* const /*srcLink*/) const {
        return myEmptyLinks;
    }

    virtual const std::vector<MSLane*>& getFoeInternalLanes(const MSLink* const /*srcLink*/) const {
        return myEmptyLanes;
    }

    inline const ConstMSEdgeVector& getIncoming() const {
        return myIncoming;
    }

    int getNrOfIncomingLanes() const;

    inline const ConstMSEdgeVector& getOutgoing() const {
        return myOutgoing;
    }

    /** @brief Returns all internal lanes on the junction
     */
    virtual const std::vector<MSLane*> getInternalLanes() const {
        return myEmptyLanes;
    }

    void addIncoming(MSEdge* edge) {
        myIncoming.push_back(edge);
    }

    void addOutgoing(MSEdge* edge) {
        myOutgoing.push_back(edge);
    }

    /// @brief return the type of this Junction
    SumoXMLNodeType getType() const {
        return myType;
    }

    /// @brief erase vehicle from myLinkLeaders
    void passedJunction(const MSVehicle* vehicle);

    // @brief return the underlying right-of-way and conflict matrix
    virtual const MSJunctionLogic* getLogic() const {
        return nullptr;
    }

protected:
    /// @brief Tye type of this junction
    SumoXMLNodeType myType;

    /// @brief The position of the junction
    Position myPosition;

    /// @brief The shape of the junction
    PositionVector myShape;

    /// @briefh The (optional) junction name
    std::string myName;

    std::vector<MSLink*> myEmptyLinks;
    std::vector<MSLane*> myEmptyLanes;


    /// @brief incoming edges
    ConstMSEdgeVector myIncoming;
    /// @brief outgoing edges
    ConstMSEdgeVector myOutgoing;

    /// @brief definition of the static dictionary type
    typedef std::map<std::string, MSJunction* > DictType;

private:
    /// @brief Invalidated copy constructor.
    MSJunction(const MSJunction&);

    /// @brief Invalidated assignment operator.
    MSJunction& operator=(const MSJunction&);

};
