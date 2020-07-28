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
/// @file    RandomDistributor.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2005-09-15
///
// Represents a generic random distribution
/****************************************************************************/
#pragma once
#include <config.h>

#include <cassert>
#include <limits>
#include <utils/common/RandHelper.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RandomDistributor
 * @brief Represents a generic random distribution
 *
 * This class allows to create random distributions by assigning
 *  arbitrary (non-negative) probabilities to its elements. The
 *  random number generator used is specified in RandHelper.
 *
 * @see RandHelper
 */

template<class T>
class RandomDistributor {
public:
    /** @brief Constructor for an empty distribution
     */
    RandomDistributor() :
        myProb(0) {
    }

    /// @brief Destructor
    ~RandomDistributor() { }

    /** @brief Adds a value with an assigned probability to the distribution.
     *
     * If the value is already member of the distribution and checkDuplicates is
     * true (the default) the given probability is added to the current.
     * The probability has to be non-negative but values larger than one are
     * allowed (and scaled accordingly when an element is drawn).
     *
     * @param[in] val The value to add to the distribution
     * @param[in] prob The probability assigned to the value
     * @return true if a new value was added, false if just the probability of an existing one was updated
     */
    bool add(T val, double prob, bool checkDuplicates = true) {
        myProb += prob;
        assert(myProb >= 0);
        if (checkDuplicates) {
            for (int i = 0; i < (int)myVals.size(); i++) {
                if (val == myVals[i]) {
                    myProbs[i] += prob;
                    assert(myProbs[i] >= 0);
                    return false;
                }
            }
        } else {
            assert(prob >= 0);
        }
        myVals.push_back(val);
        myProbs.push_back(prob);
        return true;
    }

    /** @brief Removes a value with an assigned probability from the distribution.
     *
     * @param[in] val The value to remove from the distribution
     * @return true if a new value was added, false if just the probability of an existing one was updated
     */
    bool remove(T val) {
        for (int i = 0; i < (int)myVals.size(); i++) {
            if (myVals[i] == val) {
                myProb -= myProbs[i];
                myProbs.erase(myProbs.begin() + i);
                myVals.erase(myVals.begin() + i);
                return true;
            }
        }
        return false;
    }

    /** @brief Draw a sample of the distribution.
     *
     * A random sample is drawn according to the assigned probabilities.
     *
     * @param[in] which The random number generator to use; the static one will be used if 0 is passed
     * @return the drawn member
     */
    T get(std::mt19937* which = 0) const {
        if (myProb == 0) {
            throw OutOfBoundsException();
        }
        double prob = RandHelper::rand(myProb, which);
        for (int i = 0; i < (int)myVals.size(); i++) {
            if (prob < myProbs[i]) {
                return myVals[i];
            }
            prob -= myProbs[i];
        }
        return myVals.back();
    }

    /** @brief Return the sum of the probabilites assigned to the members.
     *
     * This should be zero if and only if the distribution is empty.
     *
     * @return the total probability
     */
    double getOverallProb() const {
        return myProb;
    }

    /// @brief Clears the distribution
    void clear() {
        myProb = 0;
        myVals.clear();
        myProbs.clear();
    }

    /** @brief Returns the members of the distribution.
     *
     * See getProbs for the corresponding probabilities.
     *
     * @return the members of the distribution
     * @see RandomDistributor::getProbs
     */
    const std::vector<T>& getVals() const {
        return myVals;
    }

    /** @brief Returns the probabilities assigned to the members of the distribution.
     *
     * See getVals for the corresponding members.
     *
     * @return the probabilities assigned to the distribution
     * @see RandomDistributor::getVals
     */
    const std::vector<double>& getProbs() const {
        return myProbs;
    }

private:
    /// @brief the total probability
    double myProb;
    /// @brief the members
    std::vector<T> myVals;
    /// @brief the corresponding probabilities
    std::vector<double> myProbs;

};
