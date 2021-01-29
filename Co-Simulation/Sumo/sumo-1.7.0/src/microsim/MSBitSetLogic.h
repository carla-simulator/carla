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
/// @file    MSBitSetLogic.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Wed, 12 Dez 2001
///
// Container for holding a right-of-way matrix
/****************************************************************************/
#pragma once
#include <config.h>

#include <bitset>
#include <vector>
#include "MSJunctionLogic.h"
#include "MSLogicJunction.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSBitSetLogic
 *
 * N is sum of the number of links of the junction's inLanes.
 */
template< int N >
class MSBitSetLogic : public MSJunctionLogic {
public:
    /** @brief Container that holds the right of way bitsets.
        Each link has it's own
        bitset. The bits in the bitsets correspond to the links. To create
        a bitset for a particular link, set the bits to true that correspond
        to links that have the right of way. All others set to false,
        including the link's "own" link-bit. */
    typedef std::vector< std::bitset< N > > Logic;

    /** @brief Container holding the information which internal lanes prohibt which links
        Build the same way as Logic */
    typedef std::vector< std::bitset< N > > Foes;


public:
    /// Use this constructor only.
    MSBitSetLogic(int nLinks,
                  Logic* logic,
                  Foes* foes,
                  std::bitset<SUMO_MAX_CONNECTIONS> conts)
        : MSJunctionLogic(nLinks), myLogic(logic),
          myInternalLinksFoes(foes), myConts(conts) {}


    /// Destructor.
    ~MSBitSetLogic() {
        delete myLogic;
        delete myInternalLinksFoes;
    }


    /// @brief Returns the response for the given link
    const MSLogicJunction::LinkBits& getResponseFor(int linkIndex) const {
        return (*myLogic)[linkIndex];
    }

    /// @brief Returns the foes for the given link
    const MSLogicJunction::LinkBits& getFoesFor(int linkIndex) const {
        return (*myInternalLinksFoes)[linkIndex];
    }

    bool getIsCont(int linkIndex) const {
        return myConts.test(linkIndex);
    }

    virtual bool hasFoes() const {
        for (typename Logic::const_iterator i = myLogic->begin(); i != myLogic->end(); ++i) {
            if ((*i).any()) {
                return true;
            }
        }
        return false;
    }

private:
    /// junctions logic based on std::bitset
    Logic* myLogic;

    /// internal lanes logic
    Foes* myInternalLinksFoes;

    std::bitset<SUMO_MAX_CONNECTIONS> myConts;

private:
    /// @brief Invalidated copy constructor.
    MSBitSetLogic(const MSBitSetLogic&);

    /// @brief Invalidated assignment operator.
    MSBitSetLogic& operator=(const MSBitSetLogic&);

};


/** To make things easier we use a fixed size. SUMO_MAX_CONNECTIONS will hopefully be sufficient even for
    large asian junctions.
    So, here comes the type which should be used by the netbuilder. */
typedef MSBitSetLogic<SUMO_MAX_CONNECTIONS> MSBitsetLogic;
