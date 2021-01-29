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
/// @file    StringTokenizer.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    ?
///
// A java-style StringTokenizer for c++ (stl)
/****************************************************************************/
#pragma once
#include <string>
#include <vector>

/**
 * StringTokenizer
 * A class similar to the StringTokenizer from Java. It splits a string at
 * the given string or character (or one of the special cases NEWLINE or
 * WHITECHAR) and allows to iterate over the so generated substrings.
 *
 * The normal usage is like this:
 * <pre>
 * StringTokenizer st(CString("This is a line"), ' ');
 * while(st.hasNext())
 *    cout << st.next() << endl;
 * </pre>
 * This would generate the output:
 * <pre>
 * This
 * is
 * a
 * line
 * </pre>
 *
 * There is something to know about the behaviour:
 * When using WHITECHAR, a list of whitechars occuring in  the string to
 * split is regarded as a single divider. All other parameter will use
 * multiple occurences of operators as a list of single divider and the
 * string between them will have a length of zero.
 */
// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class StringTokenizer {
public:
    /// @brief identifier for splitting the given string at all newline characters
    static const int NEWLINE;

    /// @brief identifier for splitting the given string at all whitespace characters
    static const int WHITECHARS;

    /// @brief the ascii index of the highest whitespace character
    static const int SPACE;

    /// @brief the ascii index of the tab character
    static const int TAB;

public:
    /// @brief default constructor
    StringTokenizer();

    /**@brief constructor
     * @param tosplit is the string to split into substrings. If the string between two split positions is empty, it will not be returned.
     * @note same as StringTokenizer(tosplit, StringTokenizer.WHITECHARS)
     */
    StringTokenizer(std::string tosplit);

    /**@brief constructor
     * @note the first string will be split at the second string's occurences.
             If the optional third parameter is true, the string will be split whenever
             a char from the second string occurs. If the string between two split
             positions is empty, it will nevertheless be returned.
     */
    StringTokenizer(std::string tosplit, std::string token, bool splitAtAllChars = false);

    /**@brief constructor
     * @note When StringTokenizer.NEWLINE is used as second parameter, the string
             will be split at all occurences of a newline character (0x0d / 0x0a)
             When StringTokenizer.WHITECHARS is used as second parameter, the
             string will be split at all characters below 0x20 (SPACE)
             All other ints specified as second parameter are casted int o a char
             at which the string will be splitted.
     */
    StringTokenizer(std::string tosplit, int special);

    /// @brief destructor
    ~StringTokenizer();

    /// @brief reinitialises the internal iterator
    void reinit();

    /// @brief returns the information whether further substrings exist
    bool hasNext();

    /// @brief returns the next substring when it exists. Otherwise the behaviour is undefined
    std::string next();

    /// @brief returns the number of existing substrings
    int size() const;

    /// @brief returns the first substring without moving the iterator
    std::string front();

    /// @brief returns the item at the given position
    std::string get(int pos) const;

    /// @brief return vector of strings
    std::vector<std::string> getVector();

private:
    /// @brief splits the first string at all occurences of the second. If the third parameter is true split at all chars given in the second
    void prepare(const std::string& tosplit, const std::string& token, bool splitAtAllChars);

    /// @brief @brief splits the first string at all occurences of whitechars
    void prepareWhitechar(const std::string& tosplit);

private:
    /// @brief a list of positions/lengths
    typedef std::vector<int> SizeVector;

    /// @brief the string to split
    std::string myTosplit;

    /// @brief the current position in the list of substrings
    int myPos;

    /// @brief the list of substring starts
    SizeVector myStarts;

    /// @brief the list of substring lengths
    SizeVector myLengths;
};
