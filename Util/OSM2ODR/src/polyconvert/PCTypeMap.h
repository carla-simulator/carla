/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    PCTypeMap.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 05 Dec 2005
///
// A storage for type mappings
/****************************************************************************/
#pragma once
#include <config.h>

#include "utils/common/RGBColor.h"
#include <string>
#include <map>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;

/**
 * @class PCTypeMap
 * @brief A storage for type mappings
 *
 * This class holds the mappings between names of read polygon/poi types and the
 *  values (color, new type name etc.) that shall be assigned to them.
 */
class PCTypeMap {
public:
    /// @brief Constructor. The default type is constructed based on the given options
    PCTypeMap(const OptionsCont& oc);


    /// @brief Destructor
    ~PCTypeMap();


    /**
     * @struct TypeDef
     * @brief A single definition of values that shall be used for a given type
     */
    struct TypeDef {
        /// @brief The new type id to use
        std::string id;
        /// @brief The color to use
        RGBColor color;
        /// @brief The prefix to use
        std::string prefix;
        /// @brief The layer to use
        double layer;
        /// @brief The angle to use
        double angle;
        /// @brief The image file to use
        std::string imgFile;
        /// @brief Information whether polygons of this type shall be discarded
        bool discard;
        /// @brief Information whether polygons of this type can be filled
        bool allowFill;

    };


    /** @brief Adds a type definition
     *
     * @param[in] id The original id of the type
     * @param[in] newid The new id (name) of the type
     * @param[in] color The color to set for imported objects of this type
     * @param[in] prefix The prefix to prepend to the read names of this type's objects
     * @param[in] layer The layer number to set for this type's objects
     * @param[in] angle The angle to rotate this type's objects
     * @param[in] imgFile The image file used as texture for objects of this type
     * @param[in] discard Whether objects of this type shall be discarded
     * @param[in] allowFill Whether objects of this type may be filled
     * @return Whether the type could been added (was not known before)
     */
    bool add(const std::string& id, const std::string& newid, const std::string& color,
             const std::string& prefix, double layer, double angle, const std::string& imgFile,
             bool discard, bool allowFill);


    /** @brief Returns a type definition
     *
     * This type definition MUST have been added otherwise the further process
     *  is undefined.
     * @param[in] id The id of the type to get the definitions of
     * @return Definition of the named type
     */
    const TypeDef& get(const std::string& id);


    /** @brief Returns the information whether the named type is known
     * @param[in] id The id of the type
     * @return Whether a definition of the named type was added before
     */
    bool has(const std::string& id);

    /// @brief get the default type according to the given options
    const TypeDef& getDefault() {
        return myDefaultType;
    }

protected:
    /// @brief A map of type names to type definitions
    std::map<std::string, TypeDef> myTypes;

    TypeDef myDefaultType;

};
