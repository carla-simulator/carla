//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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

/**
 * @mainpage %Veins - The open source vehicular network simulation framework.
 *
 * See the %Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 */

#pragma once

#include <memory>
#include <utility>

#include <omnetpp.h>

// Version number of last release ("major.minor.patch") or an alpha version, if nonzero
#define VEINS_VERSION_MAJOR 5
#define VEINS_VERSION_MINOR 0
#define VEINS_VERSION_PATCH 0
#define VEINS_VERSION_ALPHA 0

// Explicitly check OMNeT++ version number
#if OMNETPP_VERSION < 0x500
#error At least OMNeT++/OMNEST version 5.0.0 required
#endif

// Import whole omnetpp namespace
namespace omnetpp {
}
using namespace omnetpp;

// VEINS_API macro. Allows us to use the same .h files for both building a .dll and linking against it
#if defined(VEINS_EXPORT)
#define VEINS_API OPP_DLLEXPORT
#elif defined(VEINS_IMPORT)
#define VEINS_API OPP_DLLIMPORT
#else
#define VEINS_API
#endif

// Macro for marking code as deprecated
#define VEINS_DEPRECATED _OPPDEPRECATED

// Convenience macros
#define RNGCONTEXT (cSimulation::getActiveSimulation()->getContext())->

/**
 * %Veins - The open source vehicular network simulation framework.
 */
namespace veins {
#ifdef __cpp_lib_make_unique
using make_unique = std::make_unique;
#else
/**
 * User-defined implementation of std::make_unique.
 *
 * Until Veins builds on C++14, this provides equivalent functionality.
 */
template <typename T, typename ... Args>
std::unique_ptr<T> make_unique(Args&& ... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
} // namespace veins
