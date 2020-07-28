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
/// @file    PCLoaderDlrNavteq.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Thu, 02.11.2006
///
// A reader of pois and polygons stored in DLR-Navteq (Elmar)-format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include "PCPolyContainer.h"
#include "PCTypeMap.h"
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
class OptionsCont;


// ===========================================================================
// class declarations
// ===========================================================================
/**
 * @class PCLoaderDlrNavteq
 * @brief A reader of pois and polygons stored in DLR-Navteq (Elmar)-format
 *
 * Reads pois stored in "pointcollection.txt" and polygons stored in
 *  "...polygons.txt"/"...water_polygons.txt", applies the given projection
 *  and network offset and stores the so build pois/polys into the given map.
 */
class PCLoaderDlrNavteq {
public:
    /** @brief Loads pois/polygons assumed to be stored as according DLR-Navteq (Elmar)-files
     *
     * If the option "elmar-poi-files" is set within the given options container,
     *  the files stored herein are parsed using "loadPOIFiles", assuming this
     *  option contains file paths to files containing pois stored in DLR-Navteq
     *  "pointcollection.txt"-format.
     *
     * If the option "elmar-poly-files" is set within the given options container,
     *  the files stored herein are parsed using "loadPolyFiles", assuming this
     *  option contains file paths to files containing polygons stored in DLR-Navteq
     *  "...polygons.txt"/"...water_polygons.txt"-format.
     *
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys/pois to
     * @param[in] tm The type map to use for setting values of loaded polys/pois
     * @exception ProcessError if something fails
     */
    static void loadIfSet(OptionsCont& oc, PCPolyContainer& toFill,
                          PCTypeMap& tm);


protected:
    /** @brief Loads pois assumed to be stored as according DLR-Navteq (Elmar)-files
     *
     * Goes through the list of files given in "elmar-poi-files". Calls
     *  "loadPOIFile" using each of these as the first parameter.
     *
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded pois to
     * @param[in] tm The type map to use for setting values of loaded pois
     * @exception ProcessError if something fails
     */
    static void loadPOIFiles(OptionsCont& oc, PCPolyContainer& toFill,
                             PCTypeMap& tm);


    /** @brief Loads polygons assumed to be stored as according DLR-Navteq (Elmar)-files
     *
     * Goes through the list of files given in "elmar-poly-files". Calls
     *  "loadPolyFile" using each of these as the first parameter.
     *
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys to
     * @param[in] tm The type map to use for setting values of loaded polys
     * @exception ProcessError if something fails
     */
    static void loadPolyFiles(OptionsCont& oc, PCPolyContainer& toFill,
                              PCTypeMap& tm);


    /** @brief Loads DLR-Navteq (Elmar)-pois from the given file
     * @param[in] file The name of the file to parse
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys to
     * @param[in] tm The type map to use for setting values of loaded polys
     * @exception ProcessError if something fails
     */
    static void loadPOIFile(const std::string& file,
                            OptionsCont& oc, PCPolyContainer& toFill,
                            PCTypeMap& tm);


    /** @brief Loads DLR-Navteq (Elmar)-polygons from the given file
     * @param[in] file The name of the file to parse
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys to
     * @param[in] tm The type map to use for setting values of loaded polys
     * @exception ProcessError if something fails
     */
    static void loadPolyFile(const std::string& file,
                             OptionsCont& oc, PCPolyContainer& toFill,
                             PCTypeMap& tm);


};
