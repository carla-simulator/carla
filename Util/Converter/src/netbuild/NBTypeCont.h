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
/// @file    NBTypeCont.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    Tue, 20 Nov 2001
///
// A storage for available types of edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/common/SUMOVehicleClass.h>
#include <netbuild/NBEdge.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBTypeCont
 * @brief A storage for available types of edges
 *
 * NBTypeCont stores properties of edge-types of edges. Additionally, a default
 *  type is stored which is used if no type information is given.
 *
 * This structure also contains a structure for determining node types using edge
 *  speeds.
 */
class NBTypeCont {
public:
    /// @brief Constructor
    NBTypeCont() {}


    /// @brief Destructor
    ~NBTypeCont() {}


    /** @brief Sets the default values
     * @param[in] defaultNumLanes The default number of lanes an edge has
     * @param[in] defaultLaneWidth The default width of lanes
     * @param[in] defaultSpeed The default speed allowed on an edge
     * @param[in] defaultPriority The default priority of an edge
     * @param[in] defaultPermissions The default permissions of an edge
     */
    void setDefaults(int defaultNumLanes, double defaultLaneWidth,
                     double defaultSpeed, int defaultPriority,
                     SVCPermissions defaultPermissions);


    /** @brief Adds a type into the list
     * @param[in] id The id of the type
     * @param[in] numLanes The number of lanes an edge of this type has
     * @param[in] maxSpeed The speed allowed on an edge of this type
     * @param[in] prio The priority of an edge of this type
     * @param[in] permissions The encoding of vehicle classes allowed on an edge of this type
     * @param[in] width The width of lanes of edgesof this type
     * @param[in] oneWayIsDefault Whether edges of this type are one-way per default
     * @return Whether the type could be added (no type with the same id existed)
     */
    void insert(const std::string& id, int numLanes,
                double maxSpeed, int prio,
                SVCPermissions permissions,
                double width, bool oneWayIsDefault,
                double sidewalkWidth,
                double bikeLaneWidth,
                double widthResolution,
                double maxWidth,
                double minWidth);

    /** @brief Returns the number of known types
     * @return The number of known edge types (excluding the default)
     */
    int size() const {
        return (int) myTypes.size();
    }


    /** @brief Returns whether the named type is in the container
     * @return Whether the named type is known
     */
    bool knows(const std::string& type) const;


    /** @brief Marks a type as to be discarded
     * @param[in] id The id of the type
     */
    bool markAsToDiscard(const std::string& id);

    /** @brief Marks an attribute of a type as set
     * @param[in] id The id of the type
     * @param[in] attr The id of the attribute
     */
    bool markAsSet(const std::string& id, const SumoXMLAttr attr);

    /** @brief Adds a restriction to a type
     * @param[in] id The id of the type
     * @param[in] svc The vehicle class the restriction refers to
     * @param[in] speed The restricted speed
     */
    bool addRestriction(const std::string& id, const SUMOVehicleClass svc, const double speed);

    /** @brief Copy restrictions to a type
     * @param[in] fromId The id of the source type
     * @param[in] toId The id of the destination type
     */
    bool copyRestrictionsAndAttrs(const std::string& fromId, const std::string& toId);

    /// @brief writes all types a s XML
    void writeTypes(OutputDevice& into) const;

    /// @name Type-dependant Retrieval methods
    /// @{

    /** @brief Returns the number of lanes for the given type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the lane number for
     * @return The number of lanes an edge of this type has
     */
    int getNumLanes(const std::string& type) const;


    /** @brief Returns the maximal velocity for the given type [m/s]
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the speed for
     * @return The allowed speed on edges of this type
     */
    double getSpeed(const std::string& type) const;


    /** @brief Returns the priority for the given type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the priority for
     * @return The priority of edges of this type
     */
    int getPriority(const std::string& type) const;


    /** @brief Returns whether edges are one-way per default for the given type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the one-way information for
     * @return Whether edges of this type are one-way per default
     * @todo There is no default for one-way!?
     */
    bool getIsOneWay(const std::string& type) const;


    /** @brief Returns the information whether edges of this type shall be discarded.
     *
     * Returns false if the type is not known.
     * @param[in] type The id of the type
     * @return Whether edges of this type shall be discarded.
     */
    bool getShallBeDiscarded(const std::string& type) const;

    /** @brief Returns the resolution for interpreting edge/lane widths of the given
     * type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the width resolution for
     * @return The width resolution on edges of this type
     */
    double getWidthResolution(const std::string& type) const;

    /** @brief Returns the maximum edge/lane widths of the given
     * type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the maximum width for
     * @return The maximum width on edges of this type
     */
    double getMaxWidth(const std::string& type) const;

    /** @brief Returns the minimum edge/lane widths of the given
     * type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the maximum width for
     * @return The minimum width on edges of this type
     */
    double getMinWidth(const std::string& type) const;


    /** @brief Returns whether an attribute of a type was set
     * @param[in] type The id of the type
     * @param[in] attr The id of the attribute
     * @return Whether the attribute was set
     */
    bool wasSet(const std::string& type, const SumoXMLAttr attr) const;


    /** @brief Returns allowed vehicle classes for the given type
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the list of allowed vehicles classes for
     * @return List of vehicles class which may use edges of the given type
     */
    SVCPermissions getPermissions(const std::string& type) const;


    /** @brief Returns the lane width for the given type [m]
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the width for
     * @return The width of lanes of edges of this type
     */
    double getWidth(const std::string& type) const;


    /** @brief Returns the lane width for a sidewalk to be added [m]
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the width for
     * @return The width of lanes of edges of this type
     */
    double getSidewalkWidth(const std::string& type) const;


    /** @brief Returns the lane width for a bike lane to be added [m]
     *
     * If the named type is not known, the default is returned
     * @param[in] type The name of the type to return the width for
     * @return The width of lanes of edges of this type
     */
    double getBikeLaneWidth(const std::string& type) const;
    /// @}


private:
    struct TypeDefinition {
        /// @brief Constructor
        TypeDefinition() :
            numLanes(1), speed((double) 13.89), priority(-1),
            permissions(SVC_UNSPECIFIED),
            oneWay(true), discard(false),
            width(NBEdge::UNSPECIFIED_WIDTH),
            widthResolution(0),
            maxWidth(0),
            minWidth(0),
            sidewalkWidth(NBEdge::UNSPECIFIED_WIDTH),
            bikeLaneWidth(NBEdge::UNSPECIFIED_WIDTH) {
        }

        /// @brief Constructor
        TypeDefinition(int _numLanes, double _speed, int _priority,
                       double _width, SVCPermissions _permissions, bool _oneWay,
                       double _sideWalkWidth,
                       double _bikeLaneWidth,
                       double _widthResolution,
                       double _maxWidth,
                       double _minWidth) :
            numLanes(_numLanes), speed(_speed), priority(_priority),
            permissions(_permissions),
            oneWay(_oneWay), discard(false),
            width(_width),
            widthResolution(_widthResolution),
            maxWidth(_maxWidth),
            minWidth(_minWidth),
            sidewalkWidth(_sideWalkWidth),
            bikeLaneWidth(_bikeLaneWidth) {
        }

        /// @brief The number of lanes of an edge
        int numLanes;
        /// @brief The maximal velocity on an edge in m/s
        double speed;
        /// @brief The priority of an edge
        int priority;
        /// @brief List of vehicle types that are allowed on this edge
        SVCPermissions permissions;
        /// @brief Whether one-way traffic is mostly common for this type (mostly unused)
        bool oneWay;
        /// @brief Whether edges of this type shall be discarded
        bool discard;
        /// @brief The width of lanes of edges of this type [m]
        double width;
        /// @brief The resolution for interpreting custom (noisy) lane widths of this type [m]
        double widthResolution;
        /// @brief The maximum width for lanes of this type [m]
        double maxWidth;
        /// @brief The minimum width for lanes of this type [m]
        double minWidth;
        /* @brief The width of the sidewalk that should be added as an additional lane
         * a value of NBEdge::UNSPECIFIED_WIDTH indicates that no sidewalk should be added */
        double sidewalkWidth;
        /* @brief The width of the bike lane that should be added as an additional lane
         * a value of NBEdge::UNSPECIFIED_WIDTH indicates that no bike lane should be added */
        double bikeLaneWidth;
        /// @brief The vehicle class specific speed restrictions
        std::map<SUMOVehicleClass, double> restrictions;
        /// @brief The attributes which have been set
        std::set<SumoXMLAttr> attrs;

    };


    /** @brief Retrieve the name or the default type
     *
     * If no name is given, the default type is returned
     * @param[in] name The name of the type to retrieve
     * @return The named type
     */
    const TypeDefinition& getType(const std::string& name) const;


private:
    /// @brief The default type
    TypeDefinition myDefaultType;

    /// @brief A container of types, accessed by the string id
    typedef std::map<std::string, TypeDefinition> TypesCont;

    /// @brief The container of types
    TypesCont myTypes;


private:
    /** @brief invalid copy constructor */
    NBTypeCont(const NBTypeCont& s);

    /** @brief invalid assignment operator */
    NBTypeCont& operator=(const NBTypeCont& s);


};
