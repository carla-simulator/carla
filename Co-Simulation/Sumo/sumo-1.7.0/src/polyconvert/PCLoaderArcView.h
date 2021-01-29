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
/// @file    PCLoaderArcView.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A reader of pois and polygons from shape files
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class PCPolyContainer;
class PCTypeMap;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PCLoaderArcView
 * @brief A reader of pois and polygons from shape files
 *
 * The current importer works only if SUMO was compiled with GDAL-support.
 *  If not, an error message is generated.
 *
 * @todo reinsert import via shapelib
 */
class PCLoaderArcView {
public:
    /** @brief Loads pois/polygons assumed to be stored as shape files-files
     *
     * If the option "shape-files" is set within the given options container,
     *  the files stored herein are parsed using "load", assuming this
     *  option contains file paths to files containing pois and polygons stored
     *  as shape-files.
     *
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys/pois to
     * @param[in] tm The type map to use for setting values of loaded polys/pois
     * @exception ProcessError if something fails
     */
    static void loadIfSet(OptionsCont& oc, PCPolyContainer& toFill,
                          PCTypeMap& tm);


protected:
    /** @brief Parses pois/polys stored within the given file
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys/pois to
     * @param[in] tm The type map to use for setting values of loaded polys/pois
     * @exception ProcessError if something fails
     */
    static void load(const std::string& file, OptionsCont& oc, PCPolyContainer& toFill,
                     PCTypeMap& tm);


private:
    /// @brief Invalidated copy constructor.
    PCLoaderArcView(const PCLoaderArcView&);

    /// @brief Invalidated assignment operator.
    PCLoaderArcView& operator=(const PCLoaderArcView&);

};
