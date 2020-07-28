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
/// @file    VectorHelper.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A simple vector of doubles
/****************************************************************************/
#pragma once
#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
template<class T>
class VectorHelper {
public:
    static T sum(const std::vector<T>& v) {
        T sum = 0;
        for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); i++) {
            sum += *i;
        }
        return sum;
    }

    static void normaliseSum(std::vector<T>& v, T msum = 1.0) {
        if (msum == 0 || v.size() == 0) {
            // is an error; do nothing
            return;
        }
        T rsum = sum(v);
        if (rsum == 0) {
            set(v, (T) 1.0 * msum / (T) v.size());
            return;
        }
        div(v, rsum / msum);
    }

    static void div(std::vector<T>& v, T by) {
        for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++) {
            *i /= by;
        }
    }

    static void removeDouble(std::vector<T>& v) {
        typename std::vector<T>::iterator i = v.begin();
        while (i != v.end()) {
            for (typename std::vector<T>::iterator j = i + 1; j != v.end();) {
                if (*i == *j) {
                    j = v.erase(j);
                } else {
                    j++;
                }
            }
            i++;
        }
    }


    static void set(std::vector<T>& v, T to) {
        for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++) {
            *i = to;
        }
    }

    static T maxValue(const std::vector<T>& v) {
        T m = -std::numeric_limits<T>::max();
        for (typename std::vector<T>::const_iterator j = v.begin() ; j != v.end(); j++) {
            if ((*j) > m) {
                m = *j;
            }
        }
        return m;
    }

    static T minValue(const std::vector<T>& v) {
        T m = std::numeric_limits<T>::max();
        for (typename std::vector<T>::const_iterator j = v.begin(); j != v.end(); j++) {
            if ((*j) < m) {
                m = *j;
            }
        }
        return m;
    }

    static void remove_smaller_than(std::vector<T>& v, T swell) {
        for (typename std::vector<T>::iterator j = v.begin(); j != v.end();) {
            if ((*j) < swell) {
                j = v.erase(j);
            } else {
                j++;
            }
        }
    }

    static void remove_larger_than(std::vector<T>& v, T swell) {
        for (typename std::vector<T>::iterator j = v.begin(); j != v.end();) {
            if ((*j) > swell) {
                j = v.erase(j);
            } else {
                j++;
            }
        }
    }

    static void add2All(std::vector<T>& v, T what) {
        for (typename std::vector<T>::iterator j = v.begin(); j != v.end(); j++) {
            (*j) += what;
        }
    }

    /// Returns the information whether at least one element is within both vectors
    static bool subSetExists(const std::vector<T>& v1, const std::vector<T>& v2) {
        for (typename std::vector<T>::const_iterator i = v1.begin(); i != v1.end(); i++) {
            int val1 = (*i);
            if (find(v2.begin(), v2.end(), val1) != v2.end()) {
                return true;
            }
        }
        return false;
    }



};

template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); i++) {
        if (i != v.begin()) {
            os << ", ";
        }
        os << (*i);
    }
    return os;
}
