//
// Copyright (C) 2018 Fabian Bronner <fabian.bronner@ccs-labs.org>
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

#include "veins/base/toolbox/Spectrum.h"

#include <sstream>

namespace veins {

Spectrum::Frequencies normalizeFrequencies(Spectrum::Frequencies freqs)
{
    // sort and deduplicate frequencies first
    std::sort(freqs.begin(), freqs.end());
    freqs.erase(std::unique(freqs.begin(), freqs.end()), freqs.end());
    return freqs;
}

Spectrum::Spectrum(Spectrum::Frequencies freqs)
    : frequencies(normalizeFrequencies(freqs))
{
}

const double& Spectrum::operator[](size_t index) const
{
    return frequencies.at(index);
}

size_t Spectrum::indexOf(double freq) const
{
    // Binary search
    auto it = std::lower_bound(frequencies.begin(), frequencies.end(), freq);
    bool found = it != frequencies.end() && (*it) == freq;

    ASSERT(found == true);

    return std::distance(frequencies.begin(), it);
}

double Spectrum::freqAt(size_t freqIndex) const
{
    return frequencies.at(freqIndex);
}

size_t Spectrum::getNumFreqs() const
{
    return frequencies.size();
}

bool operator==(const Spectrum& lhs, const Spectrum& rhs)
{
    return lhs.frequencies == rhs.frequencies;
}

std::ostream& operator<<(std::ostream& os, const Spectrum& s)
{
    os << "Spectrum(";
    std::ostringstream ss;
    for (auto&& frequency : s.frequencies) {
        if (ss.tellp() != 0) {
            ss << ", ";
        }
        ss << frequency;
    }
    os << ss.str();
    os << ")";
    return os;
}
} // namespace veins
