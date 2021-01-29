//
// Copyright (C) 2019 Michele Segata <segata@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#pragma once

#include <cstddef>
#include <string>
#include <sstream>

namespace veins {

class VEINS_API ParBuffer {
public:
    ParBuffer()
        : SEP(':')
    {
    }
    ParBuffer(std::string buf)
        : SEP(':')
    {
        inBuffer = buf;
    }

    template <typename T>
    ParBuffer& operator<<(const T& v)
    {
        if (outBuffer.str().length() == 0)
            outBuffer << v;
        else
            outBuffer << SEP << v;
        return *this;
    }

    std::string next()
    {
        std::string value;
        size_t sep;

        if (inBuffer.size() == 0) return "";

        sep = inBuffer.find(SEP);
        if (sep == std::string::npos) {
            value = inBuffer;
            inBuffer = "";
        }
        else {
            value = inBuffer.substr(0, sep);
            inBuffer = inBuffer.substr(sep + 1);
        }
        return value;
    }

    ParBuffer& operator>>(double& v)
    {
        std::string value = next();
        sscanf(value.c_str(), "%lf", &v);
        return *this;
    }

    ParBuffer& operator>>(int& v)
    {
        std::string value = next();
        sscanf(value.c_str(), "%d", &v);
        return *this;
    }

    ParBuffer& operator>>(std::string& v)
    {
        v = next();
        return *this;
    }

    void set(std::string buf)
    {
        inBuffer = buf;
    }
    void clear()
    {
        outBuffer.clear();
    }
    std::string str() const
    {
        return outBuffer.str();
    }

private:
    const char SEP;
    std::stringstream outBuffer;
    std::string inBuffer;
};

} // namespace veins
