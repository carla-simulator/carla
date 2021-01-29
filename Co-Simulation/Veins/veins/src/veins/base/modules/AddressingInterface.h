//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

/*
 * AddressingInterface.h
 *
 *  Created on: Sep 9, 2010
 *      Author: Karl Wessel
 */

#pragma once

#include "veins/veins.h"

#include "veins/base/utils/SimpleAddress.h"

namespace veins {

/**
 * @brief Interface for modules which assign L2 and L3 addresses for modules.
 *
 * This interface is implemented by ARP modules which rely on a certain
 * addressing scheme to work.
 * @see ArpHost for an example which demands the module's host's index as L2 and
 * L3 address to be able to provide a simplified ARP which is useful for
 * debugging purposes.
 *
 * @see BaseARP
 * @see ArpHost
 *
 * @ingroup netwLayer
 * @ingroup baseModules
 *
 * @author Karl Wessel
 */
class VEINS_API AddressingInterface {
public:
    /** @brief Returns the L2 address for the passed mac.*/
    virtual LAddress::L2Type myMacAddr(const cModule* mac) const = 0;

    /** @brief Returns the L3 address for the passed net.*/
    virtual LAddress::L3Type myNetwAddr(const cModule* netw) const = 0;
};

} // namespace veins
