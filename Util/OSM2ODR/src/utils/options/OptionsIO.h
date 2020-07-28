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
/// @file    OptionsIO.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
///
// Helper for parsing command line arguments and reading configuration files
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class OptionsIO
 *
 * Helping methods for parsing of command line arguments and reading a
 *  configuration file.
 * Any errors are reported by throwing a ProcessError exception which
 *  contains a description about the failure.
 */
class OptionsIO {
public:
    /** @brief Stores the command line arguments for later parsing
     *
     * @param[in] argc number of arguments given at the command line
     * @param[in] argv arguments given at the command line
     */
    static void setArgs(int argc, char** argv);

    /** @brief Stores faked command line arguments for later parsing
    *
    * @param[in] args arguments given as substitute for the command line
    */
    static void setArgs(const std::vector<std::string>& args);

    /** @brief Return the number of command line arguments
     */
    static int getArgC() {
        return myArgC;
    }


    /** @brief Parses the command line arguments and loads the configuration
     *
     * Command line arguments are parsed, first, throwing a ProcessError
     *  if something fails. Then options are reset to being writeable and the
     *  configuration is loaded using "loadConfiguration". After this,
     *  the options are reset again and the command line arguments are
     *  reparsed.
     *
     * This workflow allows to read the name of a configuration file from
     *  command line arguments, first, then to load values from this configuration
     *  file and reset them by other values from the command line.
     */
    static void getOptions(const bool commandLineOnly = false);


    /** @brief Loads and parses the configuration
     *
     * The name of the configuration file is extracted from the global
     *  OptionsCont ("configuration-file" is used as the name of the option to get
     *  the name of the configuration).
     */
    static void loadConfiguration();


    /** @brief Retrieves the XML root element of a supposed configuration or net
     *
     * @param[in] filename the XML file to parse
     * @return the root element if any
     */
    static std::string getRoot(const std::string& filename);


private:
    static int myArgC;
    static char** myArgV;

};
