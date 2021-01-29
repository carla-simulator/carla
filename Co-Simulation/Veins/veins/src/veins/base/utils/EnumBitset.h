//
// Copyright (C) 2018 Dominik S. Buse <buse@ccs-labs.org>
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

#include <bitset>
#include <type_traits>

#include "veins/veins.h"

namespace veins {

/*
 * Helper struct to define the maximum element of a given Enum.
 *
 * Provide ``max'' member for every enum type that should be used as an
 * underlying type for a EnumBitset.
 */
template <typename T>
struct VEINS_API EnumTraits;

/*
 * Bitset template for scoped (class) enums.
 *
 * Wrapper around the original std::bitset to use a user-defined scoped
 * enum as the type for the bitset.
 * Allows implementation of typesafe sets.
 */
template <typename T>
class VEINS_API EnumBitset {
private:
    using EnumUnderlyingType = typename std::underlying_type<T>::type;
    std::bitset<static_cast<EnumUnderlyingType>(EnumTraits<T>::max) + 1> bits;

    EnumUnderlyingType get_value(T v) const
    {
        return static_cast<EnumUnderlyingType>(v);
    }

public:
    EnumBitset()
        : bits()
    {
    }

    explicit EnumBitset(EnumUnderlyingType val)
        : bits(val)
    {
    }

    EnumBitset(T val)
        : bits()
    {
        set(std::move(val));
    }

    EnumBitset(std::initializer_list<T> vals)
        : bits()
    {
        for (auto val : vals) {
            set(val);
        }
    }

    bool operator==(const EnumBitset<T>& rhs) const
    {
        return bits == rhs.bits;
    }

    bool operator!=(const EnumBitset<T>& rhs) const
    {
        return bits != rhs.bits;
    }

    constexpr bool operator[](std::size_t pos) const
    {
        return bits[pos];
    }

    bool test(T pos) const
    {
        return bits.test(get_value(pos));
    }

    EnumBitset& set(T pos, bool value = true)
    {
        bits.set(get_value(pos), value);
        return *this;
    }

    EnumBitset& flip(T pos)
    {
        bits.flip(get_value(pos));
        return *this;
    }

    std::string to_string() const
    {
        return bits.to_string();
    }

    unsigned long to_ulong() const
    {
        return bits.to_ulong();
    }

    unsigned long long to_ullong() const
    {
        return bits.to_ullong();
    }

    friend EnumBitset<T> operator|(EnumBitset<T> lhs, EnumBitset<T> rhs)
    {
        EnumBitset<T> ret;
        ret.bits = lhs.bits | rhs.bits;
        return ret;
    }
};

template <typename T>
EnumBitset<T> operator|(T lhs, T rhs)
{
    return EnumBitset<T>(lhs) | EnumBitset<T>(rhs);
}

} // namespace veins
