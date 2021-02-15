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
/// @file    ParBuffer.h
/// @author  Michele Segata
/// @date    Wed, 18 Apr 2012
///
// Class for the string serialization and deserialization of parameters
/****************************************************************************/

#pragma once

#include <cstddef>
#include <string>
#include <sstream>
#include <algorithm>

class ParBuffer {
public:
    ParBuffer() : SEP(':'), ESC('\\'), QUO('"'), was_empty(false) {}
    ParBuffer(std::string buf) : SEP(':'), ESC('\\'), QUO('"'),
        was_empty(false) {
        inBuffer = buf;
    }

    template<typename T> ParBuffer& operator <<(const T& v) {
        std::stringstream ss;
        std::string str_value;
        ss << v;
        str_value = escape(ss.str());
        if (outBuffer.str().length() == 0) {
            outBuffer << str_value;
        } else {
            outBuffer << SEP << str_value;
        }
        return *this;
    }

    size_t next_escape(std::string str, size_t pos) {
        size_t c_pos = str.find(SEP, pos);
        size_t e_pos = str.find(ESC, pos);
        if (c_pos == std::string::npos) {
            return e_pos;
        }
        if (e_pos == std::string::npos) {
            return c_pos;
        }
        return std::min(c_pos, e_pos);
    }

    std::string escape(std::string str) {
        size_t pos, last_pos = 0;
        std::stringstream escaping;
        std::string escaped;
        while ((pos = next_escape(str, last_pos)) != std::string::npos) {
            escaping << str.substr(last_pos, pos - last_pos);
            escaping << ESC << str.substr(pos, 1);
            last_pos = pos + 1;
        }
        if (last_pos != str.size()) {
            escaping << str.substr(last_pos);
        }
        escaped = escaping.str();
        if (escaped.empty() || (escaped.c_str()[0] == QUO && escaped.c_str()[escaped.length() - 1] == QUO)) {
            escaping.str("");
            escaping.clear();
            escaping << QUO << escaped << QUO;
            escaped = escaping.str();
        }
        return escaped;
    }

    std::string unescape(std::string str) {
        size_t pos, last_pos = 0;
        std::stringstream unescaped;
        std::string escaped;
        if (str.c_str()[0] == QUO && str.c_str()[str.length() - 1] == QUO) {
            str = str.substr(1, str.length() - 2);
        }
        while ((pos = str.find(ESC, last_pos)) != std::string::npos) {
            unescaped << str.substr(last_pos, pos - last_pos);
            unescaped << str.substr(pos + 1, 1);
            last_pos = pos + 2;
        }
        if (last_pos != str.size()) {
            unescaped << str.substr(last_pos);
        }
        return unescaped.str();
    }

    std::string next() {
        if (inBuffer.size() == 0) {
            return "";
        }

        size_t sep = std::string::npos;
        do {
            sep = inBuffer.find(SEP, sep + 1);
        } while (!(sep == std::string::npos || sep == 0 || inBuffer.c_str()[sep - 1] != ESC));

        std::string value;
        if (sep == std::string::npos) {
            value = unescape(inBuffer);
            inBuffer = "";
        } else {
            value = unescape(inBuffer.substr(0, sep));
            inBuffer = inBuffer.substr(sep + 1);
        }
        return value;
    }

    template <typename T> ParBuffer& operator>>(T& v) {
        std::string value = next();
        std::stringstream ss(value);
        ss >> v;
        // stringstream doesn't write to v if value is an empty string. the
        // only solution is letting the user know that the last parsed
        // portion was empty
        if (value == "") {
            was_empty = true;
        } else {
            was_empty = false;
        }
        return *this;
    }

    bool last_empty() {
        return was_empty;
    }

    void set(std::string buf) {
        inBuffer = buf;
    }
    void clear() {
        outBuffer.clear();
    }
    std::string str() const {
        return outBuffer.str();
    }

private:
    const char SEP;
    const char ESC;
    const char QUO;
    std::stringstream outBuffer;
    std::string inBuffer;
    bool was_empty;

};

