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
/// @file    ODDistrictCont.h
/// @author  Daniel Krajzewicz
/// @author  Yun-Pang Floetteroed
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A container for districts
/****************************************************************************/
#pragma once
#include <config.h>

#include "ODDistrict.h"
#include <utils/common/NamedObjectCont.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ODDistrictCont
 * @brief A container for districts
 *
 * Besides the inherited methods for adding/removing districts, this container
 *  allows to retrieve a random source or sink from a named district.
 */
class ODDistrictCont : public NamedObjectCont<ODDistrict*> {
public:
    /// Constructor
    ODDistrictCont();


    /// Destructor
    ~ODDistrictCont();


    /** @brief Returns the id of a random source from the named district
     *
     * At first, the named district is retrieved. If this fails, an
     *  InvalidArgument-exception is thrown. Otherwise, a source (edge)
     *  is chosen randomly from this  district using this district's
     *  getRandomSource-method which throws an OutOfBoundsException-exception
     *  if this district does not contain a source.
     *
     * @param[in] name The id of the district to get a random source from
     * @return The id of a randomly chosen source
     * @exception InvalidArgument If the named district is not known
     * @exception OutOfBoundsException If the named district has no sources
     * @see ODDistrict::getRandomSource
     */
    std::string getRandomSourceFromDistrict(const std::string& name) const;


    /** @brief Returns the id of a random sink from the named district
     *
     * At first, the named district is retrieved. If this fails, an
     *  InvalidArgument-exception is thrown. Otherwise, a sink (edge)
     *  is chosen randomly from this district using this district's
     *  getRandomSink-method which throws an OutOfBoundsException-exception
     *  if this district does not contain a sink.
     *
     * @param[in] name The id of the district to get a random sink from
     * @return The id of a randomly chosen sink
     * @exception InvalidArgument If the named district is not known
     * @exception OutOfBoundsException If the named district has no sinks
     * @see ODDistrict::getRandomSink
     */
    std::string getRandomSinkFromDistrict(const std::string& name) const;

    /// @brief load districts from files
    void loadDistricts(std::vector<std::string> files);

    /// @brief create districts from description
    void makeDistricts(const std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >& districts);

private:
    /// @brief invalidated copy constructor
    ODDistrictCont(const ODDistrictCont& s);

    /// @brief invalidated assignment operator
    ODDistrictCont& operator=(const ODDistrictCont& s);


};
