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
/// @file    StringUtils.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Robert Hilbrich
/// @date    unknown
///
// Some static methods for string processing
/****************************************************************************/
#pragma once
#include <string>
#include <xercesc/util/XMLString.hpp>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class StringUtils
 * @brief Some static methods for string processing
 */
class StringUtils {
public:
    /// Removes trailing and leading whitechars
    static std::string prune(const std::string& str);

    /// Transfers the content to lower case
    static std::string to_lower_case(std::string str);

    /// Transfers from Latin 1 (ISO-8859-1) to UTF-8
    static std::string latin1_to_utf8(std::string str);

    /// Converts german "Umlaute" to their latin-version
    static std::string convertUmlaute(std::string str);

    /** Replaces all occurences of the second string by the third
        string within the first string */
    static std::string replace(std::string str, const char* what,
                               const char* by);

    /** Replaces an environment variable with its value (similar to bash);
        syntax for a variable is ${NAME} */
    static std::string substituteEnvironment(std::string str);

    /// Builds a time string (hh:mm:ss) from the given seconds
    static std::string toTimeString(int time);

    /// Checks whether a given string starts with the prefix
    static bool startsWith(const std::string& str, const std::string prefix);

    /// Checks whether a given string ends with the suffix
    static bool endsWith(const std::string& str, const std::string suffix);

    /**
     * @brief Replaces the standard escapes by their XML entities.
     *
     * The strings &, <, >, ", and ' are replaced by &amp;, &lt;, &gt;, &quot;, and &apos;
     *
     * @param[in] orig The original string
     * @param[in] maskDoubleHyphen Whether -- in input shall be converted to &#45;&#45; (semantically equivalent but allowed in XML comments)
     * @return the string with the escaped sequences
     */
    static std::string escapeXML(const std::string& orig, const bool maskDoubleHyphen = false);

    /// An empty string
    static std::string emptyString;

    // the following methods stem from http://bogomip.net/blog/cpp-url-encoding-and-decoding/

    static std::string urlEncode(const std::string& url, const std::string encodeWhich = "");
    static std::string urlDecode(const std::string& encoded);

    static std::string charToHex(unsigned char c);
    static unsigned char hexToChar(const std::string& str);

    /**@brief converts a string into the integer value described by it by calling the char-type converter, which
     * @throw an EmptyData - exception if the given string is empty
     * @throw NumberFormatException - exception when the string does not contain an integer
     */
    static int toInt(const std::string& sData);

    /// @brief converts a string into the integer value described by it
    /// @return the default value if the data is empty
    static int toIntSecure(const std::string& sData, int def);

    /**@brief converts a string into the long value described by it by calling the char-type converter, which
     * @throw an EmptyData - exception if the given string is empty
     * @throw NumberFormatException - exception when the string does not contain a long integer
     */
    static long long int toLong(const std::string& sData);

    /**@brief converts a string with a hex value into the integer value described by it by calling the char-type converter
     * @throw an EmptyData - exception if the given string is empty
     * @throw a NumberFormatException - exception when the string does not contain an integer
     */
    static int hexToInt(const std::string& sData);

    /**@brief converts a string into the double value described by it by calling the char-type converter
     * @throw an EmptyData - exception if the given string is empty
     * @throw a NumberFormatException - exception when the string does not contain a double
     */
    static double toDouble(const std::string& sData);

    /// @brief converts a string into the integer value described by it
    /// @return the default value if the data is empty
    static double toDoubleSecure(const std::string& sData, const double def);

    /**@brief converts a string into the bool value described by it by calling the char-type converter
     * @return true if the data* is one of the following (case insensitive): '1', 'x', 'true', 'yes', 'on', 't'
     * @return false if the data* is one of the following (case insensitive): '0', '-', 'false', 'no', 'off', 'f'
     * @throw EmptyData - exception if the given string is empty or 0 pointer
     * @throw BoolFormatException in any other case
     */
    static bool toBool(const std::string& sData);

    /**@brief converts a 0-terminated XMLCh* array (usually UTF-16, stemming from Xerces) into std::string in UTF-8
     * @throw an EmptyData - exception if the given pointer is 0
     */
    static inline std::string transcode(const XMLCh* const data) {
        return transcode(data, (int)XERCES_CPP_NAMESPACE::XMLString::stringLen(data));
    }

    /**@brief converts a 0-terminated XMLCh* array (usually UTF-16, stemming from Xerces) into std::string in UTF-8 considering the given length
     * @throw EmptyData if the given pointer is 0
     */
    static std::string transcode(const XMLCh* const data, int length);
};
