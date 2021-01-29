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
/// @file    NamedColumnsParser.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Fri, 19 Jul 2002
///
// A parser to retrieve information from a table with known columns
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include <utils/common/StringTokenizer.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NamedColumnsParser
 * @brief A parser to retrieve information from a table with known columns
 *
 * When initialised, this parser stores the given information about the
 * order of the named elements and allows the retrieval of lines using the
 * names of these elements.
 * Use it like this:
 * @arg Initialise with "Name;PositionX;PositionY"
 *      (defDelim=default=";")
 *      (lineDelim=default=";")
 * @arg Parse each line of a table using "parseLine" (parseLine("Dummy;0;0"))
 * @arg get values using operations like: string posX = get("PositionX");
 *
 * @todo What happens if an uninitialised NamedColumnsParser is used? exceptions?
 */
class NamedColumnsParser {
public:
    /** @brief Constructor
     *
     * Does nothing, a later call to reinit is necessary
     */
    NamedColumnsParser();


    /** @brief Constructor
     *
     * Initialises the parser (mainly using "reinitMap", only "ignoreCase" and
     *  "lineDelim" are saved directly into member variables - "reinit"
     *  does the same).
     *
     * @param[in] def The line that describes (names) the entries
     * @param[in] defDelim Delimiter for the entry names
     * @param[in] lineDelim Delimiter used within data lines
     * @param[in] chomp Whether the lines shall be trimmed (white spaces shall be removed)
     * @param[in] ignoreCase Whether the case shall be ignored when parsing the definitions
     */
    NamedColumnsParser(const std::string& def, const std::string& defDelim = ";",
                       const std::string& lineDelim = ";", bool chomp = false,
                       bool ignoreCase = true);


    /// @brief Destructor
    ~NamedColumnsParser();


    /** @brief Reinitialises the parser
     *
     * Initialises the parser (mainly using "reinitMap", only "ignoreCase" and
     *  "lineDelim" are saved directly into member variables
     *
     * @param[in] def The line that describes (names) the entries
     * @param[in] defDelim Delimiter for the entry names
     * @param[in] lineDelim Delimiter used within data lines
     * @param[in] chomp Whether the lines shall be trimmed (white spaces shall be removed)
     * @param[in] ignoreCase Whether the case shall be ignored when parsing the definitions
     */
    void reinit(const std::string& def, const std::string& defDelim = ";",
                const std::string& lineDelim = ";", bool chomp = false,
                bool ignoreCase = true);


    /** @brief Parses the contents of the line
     *
     * Parses the line by tokenizing it using a StringTokenizer and the set
     *  line delimiter ("myLineDelimiter"). Stores the tokenized line into
     *  "myLineParser"
     *
     * @param[in] line The line to parse
     */
    void parseLine(const std::string& line);


    /** @brief Returns the named information
     *
     * Tries to find the given variable name within the parsed definition line
     *  ("myDefinitionsMap"). If the value was not within the definition, an
     *  UnknownElement exception is thrown. Otherwise, the method tries to return
     *  the element from the parsed value line that is at the obtained position.
     *  If the value line had less tokens than the position assumes, an OutOfBoundsException
     *  is thrown, otherwise the value at the position returned (optionally prunned).
     *
     * @param[in] name The name of the value to retrieve
     * @param[in] prune Whether the returned value shall be trimmed (leading/trainling spaces removed)
     * @return The obtained value
     * @exception UnknownElement when the element was not named during the initialisation
     * @exception OutOfBoundsException when the line was too short and did not contain the item */
    std::string get(const std::string& name,
                    bool prune = false) const;


    /** @brief Returns the information whether the named column is known
     *
     * @param[in] name The name of the value to check
     * @return Whether the named value is stored in the parsed line
     */
    bool know(const std::string& name) const;


    /** @brief Returns whether the number of named columns matches the actual number
     *
     * @return Whether the number of named columns matches the actual number
     */
    bool hasFullDefinition() const;


private:
    /** @brief Rebuilds the map of attribute names to their positions in a table
     *
     * The given definition string is split using the given delimiter. The obtained
     *  tokens are stired in "" together with their positions within the tokenized
     *  string.
     * If wished (myAmCaseInsensitive==true), the definition string is converted
     *  into lower case, first. Also, if chomp==true, each token ist prunned.
     *
     * @param[in] def The definition string
     * @param[in] delim The delimiter string
     * @param[in] chomp Whether the tokens shall be prunned
     */
    void reinitMap(std::string def, const std::string& delim = ";",
                   bool chomp = false);


    /** @brief Prunes the given string if it shall be done
     *
     * If prune==true, the given string is prunned (all leading/trailing spaces
     *  are removed).
     *
     * @param[in, out] str The string to prune (optionally)
     * @param[in] prune Whether the string shall be prunned
     */
    void checkPrune(std::string& str, bool prune) const;


private:
    /** @brief The map's definition of column item names to their positions within the table */
    typedef std::map<std::string, int> PosMap;

    /// @brief The map of column item names to their positions within the table
    PosMap myDefinitionsMap;

    /// @brief The delimiter to split the column items on
    std::string myLineDelimiter;

    /// @brief The contents of the current line
    StringTokenizer myLineParser;

    /// @brief Information whether case insensitive match shall be done
    bool myAmCaseInsensitive;

};
