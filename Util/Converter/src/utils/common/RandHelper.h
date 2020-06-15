/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    RandHelper.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Fri, 29.04.2005
///
//
/****************************************************************************/
#pragma once
#include <cassert>
#include <vector>
#include <map>
#include <random>
#include <sstream>
#include <iostream>

//#define DEBUG_RANDCALLS

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RandHelper
 * @brief Utility functions for using a global, resetable random number generator
 */
class RandHelper {
public:
    /// @brief Initialises the given options container with random number options
    static void insertRandOptions();

    /// @brief Initialises the random number generator with hardware randomness or seed
    static void initRand(std::mt19937* which = 0, const bool random = false, const int seed = 23423);

    /// @brief Reads the given random number options and initialises the random number generator in accordance
    static void initRandGlobal(std::mt19937* which = 0);

    /// @brief Returns a random real number in [0, 1)
    static inline double rand(std::mt19937* rng = 0) {
        if (rng == 0) {
            rng = &myRandomNumberGenerator;
        }
        const double res = double((*rng)() / 4294967296.0);
#ifdef DEBUG_RANDCALLS
        myCallCount[rng]++;
        if (myCallCount[rng] == myDebugIndex) {
            std::cout << "DEBUG\n"; // for setting breakpoint
        }
        std::stringstream stream; // to reduce output interleaving from different threads
        stream << " rng" << myRngId.find(rng)->second << " rand call=" << myCallCount[rng] << " val=" << res << "\n";
        std::cout << stream.str();
#endif
        return res;
    }

    /// @brief Returns a random real number in [0, maxV)
    static inline double rand(double maxV, std::mt19937* rng = 0) {
        return maxV * rand(rng);
    }

    /// @brief Returns a random real number in [minV, maxV)
    static inline double rand(double minV, double maxV, std::mt19937* rng = 0) {
        return minV + (maxV - minV) * rand(rng);
    }

    /// @brief Returns a random integer in [0, maxV-1]
    static inline int rand(int maxV, std::mt19937* rng = 0) {
        if (rng == 0) {
            rng = &myRandomNumberGenerator;
        }
        unsigned int usedBits = maxV - 1;
        usedBits |= usedBits >> 1;
        usedBits |= usedBits >> 2;
        usedBits |= usedBits >> 4;
        usedBits |= usedBits >> 8;
        usedBits |= usedBits >> 16;

        // Draw numbers until one is found in [0, maxV-1]
        int result;
        do {
            result = (*rng)() & usedBits;
        } while (result >= maxV);
        return result;
    }

    /// @brief Returns a random integer in [minV, maxV-1]
    static inline int rand(int minV, int maxV, std::mt19937* rng = 0) {
        return minV + rand(maxV - minV, rng);
    }

    /// @brief Returns a random 64 bit integer in [0, maxV-1]
    static inline long long int rand(long long int maxV, std::mt19937* rng = 0) {
        if (maxV <= std::numeric_limits<int>::max()) {
            return rand((int)maxV, rng);
        }
        if (rng == 0) {
            rng = &myRandomNumberGenerator;
        }
        unsigned long long int usedBits = maxV - 1;
        usedBits |= usedBits >> 1;
        usedBits |= usedBits >> 2;
        usedBits |= usedBits >> 4;
        usedBits |= usedBits >> 8;
        usedBits |= usedBits >> 16;
        usedBits |= usedBits >> 32;

        // Draw numbers until one is found in [0, maxV-1]
        long long int result;
        do {
            result = (((unsigned long long int)(*rng)() << 32) | (*rng)()) & usedBits;    // toss unused bits to shorten search
        } while (result >= maxV);
        return result;
    }

    /// @brief Returns a random 64 bit integer in [minV, maxV-1]
    static inline long long int rand(long long int minV, long long int maxV, std::mt19937* rng = 0) {
        return minV + rand(maxV - minV, rng);
    }

    /// @brief Access to a random number from a normal distribution
    static inline double randNorm(double mean, double variance, std::mt19937* rng = 0) {
        // Polar method to avoid cosine
        double u, q;
        do {
            u = rand(2.0, rng) - 1;
            const double v = rand(2.0, rng) - 1;
            q = u * u + v * v;
        } while (q == 0.0 || q >= 1.0);
        return (double)(mean + variance * u * sqrt(-2 * log(q) / q));
    }

    /// @brief Returns a random element from the given vector
    template<class T>
    static inline const T&
    getRandomFrom(const std::vector<T>& v, std::mt19937* rng = 0) {
        assert(v.size() > 0);
        return v[rand((int)v.size(), rng)];
    }

    /// @brief save rng state to string
    static std::string saveState(std::mt19937* rng = 0) {
        if (rng == 0) {
            rng = &myRandomNumberGenerator;
        }
        std::ostringstream oss;
        oss << (*rng);
        return oss.str();
    }

    /// @brief load rng state from string
    static void loadState(const std::string& state, std::mt19937* rng = 0) {
        if (rng == 0) {
            rng = &myRandomNumberGenerator;
        }
        std::istringstream iss(state);
        iss >> (*rng);
    }


protected:
    /// @brief the random number generator to use
    static std::mt19937 myRandomNumberGenerator;

#ifdef DEBUG_RANDCALLS
    static std::map<std::mt19937*, int> myCallCount;
    static std::map<std::mt19937*, int> myRngId;
    static int myDebugIndex;
#endif

};
