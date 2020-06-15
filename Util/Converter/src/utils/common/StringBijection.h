/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
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
/// @file    StringBijection.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// Bidirectional map between string and something else
/****************************************************************************/
#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <utils/common/UtilExceptions.h>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * Template container for maintaining a bidirectional map between strings and something else
 * It is not always a bijection since it allows for duplicate entries on both sides if either
 * checkDuplicates is set to false in the constructor or the insert function or if
 * the addAlias function is used.
 */

template< class T  >
class StringBijection {

public:

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4510 4512 4610) // no default constructor and no assignment operator; conflicts with initializer
#endif
    struct Entry {
        const char* str;
        const T key;
    };
#ifdef _MSC_VER
#pragma warning(pop)
#endif


    StringBijection() {}


    StringBijection(Entry entries[], T terminatorKey, bool checkDuplicates = true) {
        int i = 0;
        do {
            insert(entries[i].str, entries[i].key, checkDuplicates);
        } while (entries[i++].key != terminatorKey);
    }


    void insert(const std::string str, const T key, bool checkDuplicates = true) {
        if (checkDuplicates) {
            if (has(key)) {
                // cannot use toString(key) because that might create an infinite loop
                throw InvalidArgument("Duplicate key.");
            }
            if (hasString(str)) {
                throw InvalidArgument("Duplicate string '" + str + "'.");
            }
        }
        myString2T[str] = key;
        myT2String[key] = str;
    }


    void addAlias(const std::string str, const T key) {
        myString2T[str] = key;
    }


    void remove(const std::string str, const T key) {
        myString2T.erase(str);
        myT2String.erase(key);
    }


    T get(const std::string& str) const {
        if (hasString(str)) {
            return myString2T.find(str)->second;
        } else {
            throw InvalidArgument("String '" + str + "' not found.");
        }
    }


    const std::string& getString(const T key) const {
        if (has(key)) {
            return myT2String.find(key)->second;
        } else {
            // cannot use toString(key) because that might create an infinite loop
            throw InvalidArgument("Key not found.");
        }
    }


    bool hasString(const std::string& str) const {
        return myString2T.count(str) != 0;
    }


    bool has(const T key) const {
        return myT2String.count(key) != 0;
    }


    int size() const {
        return (int)myString2T.size();
    }


    std::vector<std::string> getStrings() const {
        std::vector<std::string> result;
        typename std::map<T, std::string>::const_iterator it; // learn something new every day
        for (it = myT2String.begin(); it != myT2String.end(); it++) {
            result.push_back(it->second);
        }
        return result;
    }


    void addKeysInto(std::vector<T>& list) const {
        typename std::map<T, std::string>::const_iterator it; // learn something new every day
        for (it = myT2String.begin(); it != myT2String.end(); it++) {
            list.push_back(it->first);
        }
    }


private:
    std::map<std::string, T> myString2T;
    std::map<T, std::string> myT2String;

};
