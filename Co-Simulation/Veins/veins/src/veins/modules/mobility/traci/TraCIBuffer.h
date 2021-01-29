//
// Copyright (C) 2006 Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins/veins.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"

namespace veins {

struct TraCICoord;

bool VEINS_API isBigEndian();

/**
 * Byte-buffer that stores values in TraCI byte-order
 */
class VEINS_API TraCIBuffer {
public:
    TraCIBuffer();
    TraCIBuffer(std::string buf);

    template <typename T>
    T read()
    {
        T buf_to_return;
        unsigned char* p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

        if (isBigEndian()) {
            for (size_t i = 0; i < sizeof(buf_to_return); ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                p_buf_to_return[i] = buf[buf_index++];
            }
        }
        else {
            for (size_t i = 0; i < sizeof(buf_to_return); ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                p_buf_to_return[sizeof(buf_to_return) - 1 - i] = buf[buf_index++];
            }
        }

        return buf_to_return;
    }

    template <typename T>
    void write(T inv)
    {
        unsigned char* p_buf_to_send = reinterpret_cast<unsigned char*>(&inv);

        if (isBigEndian()) {
            for (size_t i = 0; i < sizeof(inv); ++i) {
                buf += p_buf_to_send[i];
            }
        }
        else {
            for (size_t i = 0; i < sizeof(inv); ++i) {
                buf += p_buf_to_send[sizeof(inv) - 1 - i];
            }
        }
    }

    void readBuffer(unsigned char* buffer, size_t size)
    {
        if (isBigEndian()) {
            for (size_t i = 0; i < size; ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                buffer[i] = buf[buf_index++];
            }
        }
        else {
            for (size_t i = 0; i < size; ++i) {
                if (eof()) throw cRuntimeError("Attempted to read past end of byte buffer");
                buffer[size - 1 - i] = buf[buf_index++];
            }
        }
    }

    template <typename T>
    T read(T& out)
    {
        out = read<T>();
        return out;
    }

    template <typename T>
    TraCIBuffer& operator>>(T& out)
    {
        out = read<T>();
        return *this;
    }

    template <typename T>
    TraCIBuffer& operator<<(const T& inv)
    {
        write(inv);
        return *this;
    }

    /**
     * @brief
     * read and check type, then read and return an item from the buffer
     */
    template <typename T>
    T readTypeChecked(int expectedTraCIType)
    {
        uint8_t read_type(read<uint8_t>());
        ASSERT(read_type == static_cast<uint8_t>(expectedTraCIType));
        return read<T>();
    }

    bool eof() const;
    void set(std::string buf);
    void clear();
    std::string str() const;
    std::string hexStr() const;

    static void setTimeType(uint8_t val)
    {
        if (val != TraCIConstants::TYPE_INTEGER && val != TraCIConstants::TYPE_DOUBLE) {
            throw cRuntimeError("Invalid time data type");
        }
        timeAsDouble = val == TraCIConstants::TYPE_DOUBLE;
    }

private:
    std::string buf;
    size_t buf_index;
    static bool timeAsDouble;
};

template <>
std::vector<std::string> TraCIBuffer::readTypeChecked(int expectedTraCIType);
template <>
void VEINS_API TraCIBuffer::write(std::string inv);
template <>
void TraCIBuffer::write(TraCICoord inv);
template <>
std::string VEINS_API TraCIBuffer::read();
template <>
TraCICoord TraCIBuffer::read();
template <>
void TraCIBuffer::write(simtime_t o);
template <>
simtime_t TraCIBuffer::read();

} // namespace veins
