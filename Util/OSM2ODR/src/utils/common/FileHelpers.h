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
/// @file    FileHelpers.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 17 Dec 2001
///
// Functions for an easier usage of files
/****************************************************************************/
#pragma once
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include "SUMOTime.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class FileHelpers
 * @brief Functions for an easier usage of files and paths
 */
class FileHelpers {
public:
    /// @name file access functions
    /// @{

    /** @brief Checks whether the given file is readable
     *
     * @param[in] path The path to the file that shall be examined
     * @return Whether the named file is readable
     */
    static bool isReadable(std::string path);

    /** @brief Checks whether the given file is a directory
     *
     * @param[in] path The path to the file that shall be examined
     * @return Whether the named file is a directory.
     */
    static bool isDirectory(std::string path);
    /// @}

    /// @name file path evaluating functions
    /// @{

    /** @brief Removes the file information from the given path
     *
     * @param[in] path The path to the file to return the folder it is located in
     * @return The directory of the named file
     */
    static std::string getFilePath(const std::string& path);

    /** @brief Add an extension to the given file path
     *
     * @param[in] path The path to the file
     * @param[in] extension new extension (with dot, example: '.xml')
     * @return the new path with extension, the same path if it already has the extension, or a empty string if path is invalid
     */
    static std::string addExtension(const std::string& path, const std::string& extension);

    /** @brief Returns the second path as a relative path to the first file
     *
     * Given the position of the configuration file, and the information where a second
     *  file is relative to the configuration file's position, we want to known where
     *  this second file can be found. This method gets the path to the configuration file
     *  (including the configuration file name) and the path to get the relative position
     *  of and returns this relative position.
     *
     * @param[in] configPath The path the configuration file (including the config's file name)
     * @param[in] path The path to the references file (relativ to configuration path)
     * @return The file's position (relative to curent working directory)
     */
    static std::string getConfigurationRelative(const std::string& configPath, const std::string& path);

    /** @brief Returns the information whether the given name represents a socket
     *
     * A file name is meant to describe a socket address if a colon is found at a position
     *  larger than one.
     *
     * @param[in] name The name of a file
     * @return Whether the name names a socket
     */
    static bool isSocket(const std::string& name);

    /** @brief Returns the information whether the given path is absolute
     *
     * A path is meant to be absolute, if
     * @arg it is a socket
     * @arg it starts with a "/" (Linux)
     * @arg it has a ':' at the second position (Windows)
     *
     * @param[in] path The path to examine
     * @return Whether the path is absolute
     */
    static bool isAbsolute(const std::string& path);

    /** @brief Returns the path from a configuration so that it is accessable from the current working directory
     *
     * If the path is absolute, it is returned. Otherwise, the file's position
     *  is computed regarding the configuration path (see getConfigurationRelative).
     *
     * @see isAbsolute
     * @see getConfigurationRelative
     * @param[in] filename The path to the file to be examined
     * @param[in] basePath The path the configuration file (including the config's file name)
     * @return The file's position
     */
    static std::string checkForRelativity(const std::string& filename, const std::string& basePath);

    /// @brief prepend the given prefix to the last path component of the given file path
    static std::string prependToLastPathComponent(const std::string& prefix, const std::string& path);

    /// @}

    /// @name binary writing functions
    /// @{

    /** @brief Writes an integer binary
     *
     * @param[in, out] strm The stream to write into
     * @param[in] value The integer to write
     * @return Reference to the stream
     */
    static std::ostream& writeInt(std::ostream& strm, int value);

    /** @brief Writes a float binary
     *
     * This method behaves differently depending on the definition of double at compile time.
     *
     * @param[in, out] strm The stream to write into
     * @param[in] value The float to write
     * @return Reference to the stream
     */
    static std::ostream& writeFloat(std::ostream& strm, double value);

    /** @brief Writes a byte binary
     *
     * @param[in, out] strm The stream to write into
     * @param[in] value The byte to write
     * @return Reference to the stream
     */
    static std::ostream& writeByte(std::ostream& strm, unsigned char value);

    /** @brief Writes a string binary
     *
     * Writes the length of the string, first, using writeInt. Writes then the string's
     *  characters.
     *
     * @see writeInt
     * @param[in, out] strm The stream to write into
     * @param[in] value The string to write
     * @return Reference to the stream
     */
    static std::ostream& writeString(std::ostream& strm, const std::string& value);

    /** @brief Writes a time description binary
     *
     * This method behaves differently depending on the definition of SUMOTime at compile time,
     *  which in turn depends on the enabling of subsecond timesteps.
     *
     * @param[in, out] strm The stream to write into
     * @param[in] value The time to write
     * @return Reference to the stream
     */
    static std::ostream& writeTime(std::ostream& strm, SUMOTime value);

    /** @brief Writes an edge vector binary
     *
     * @param[in, out] os The stream to write into
     * @param[in] edges The edges to write
     * @return Reference to the stream
     */
    template <typename E>
    static std::ostream& writeEdgeVector(std::ostream& os, const std::vector<E>& edges);

    /** @brief Reads an edge vector binary
     *
     * @param[in] is The stream to read from
     * @param[out] edges The edge vector to write into
     * @return Reference to the stream
     */
    template <typename E>
    static void readEdgeVector(std::istream& in, std::vector<const E*>& edges, const std::string& rid);
    /// @}
};


template <typename E>
std::ostream& FileHelpers::writeEdgeVector(std::ostream& os, const std::vector<E>& edges) {
    FileHelpers::writeInt(os, (int)edges.size());
    std::vector<int> follow;
    int maxFollow = 0;
    E prev = edges.front();
    for (typename std::vector<E>::const_iterator i = edges.begin() + 1; i != edges.end(); ++i) {
        int idx = 0;
        for (; idx < prev->getNumSuccessors(); ++idx) {
            if (idx > 15) {
                break;
            }
            if (prev->getSuccessors()[idx] == (*i)) {
                follow.push_back(idx);
                if (idx > maxFollow) {
                    maxFollow = idx;
                }
                break;
            }
        }
        if (idx > 15 || idx == prev->getNumSuccessors()) {
            follow.clear();
            break;
        }
        prev = *i;
    }
    if (follow.empty()) {
        for (typename std::vector<E>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            FileHelpers::writeInt(os, (*i)->getNumericalID());
        }
    } else {
        const int bits = maxFollow > 3 ? 4 : 2;
        const int numFields = 8 * sizeof(int) / bits;
        FileHelpers::writeInt(os, -bits);
        FileHelpers::writeInt(os, edges.front()->getNumericalID());
        int data = 0;
        int field = 0;
        for (std::vector<int>::const_iterator i = follow.begin(); i != follow.end(); ++i) {
            data |= *i;
            field++;
            if (field == numFields) {
                FileHelpers::writeInt(os, data);
                data = 0;
                field = 0;
            } else {
                data <<= bits;
            }
        }
        if (field > 0) {
            FileHelpers::writeInt(os, data << ((numFields - field - 1) * bits));
        }
    }
    return os;
}


template <typename E>
void FileHelpers::readEdgeVector(std::istream& in, std::vector<const E*>& edges, const std::string& rid) {
    int size;
    in.read((char*) &size, sizeof(int));
    edges.reserve(size);
    int bitsOrEntry;
    in.read((char*) &bitsOrEntry, sizeof(int));
    if (bitsOrEntry < 0) {
        const int bits = -bitsOrEntry;
        const int numFields = 8 * sizeof(int) / bits;
        const int mask = (1 << bits) - 1;
        int edgeID;
        in.read((char*) &edgeID, sizeof(int));
        const E* prev = E::getAllEdges()[edgeID];
        assert(prev != 0);
        edges.push_back(prev);
        size--;
        int data = 0;
        int field = numFields;
        for (; size > 0; size--) {
            if (field == numFields) {
                in.read((char*) &data, sizeof(int));
                field = 0;
            }
            int followIndex = (data >> ((numFields - field - 1) * bits)) & mask;
            if (followIndex >= prev->getNumSuccessors()) {
                throw ProcessError("Invalid follower index in route '" + rid + "'!");
            }
            prev = prev->getSuccessors()[followIndex];
            edges.push_back(prev);
            field++;
        }
    } else {
        while (size > 0) {
            const E* edge = E::getAllEdges()[bitsOrEntry];
            if (edge == 0) {
                throw ProcessError("An edge within the route '" + rid + "' is not known!");
            }
            edges.push_back(edge);
            size--;
            if (size > 0) {
                in.read((char*) &bitsOrEntry, sizeof(int));
            }
        }
    }
}
