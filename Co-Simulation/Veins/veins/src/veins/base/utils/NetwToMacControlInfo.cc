//
// Copyright (C) 2011 Fraunhofer-Gesellschaft, Germany.
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

// author:      Michael Lindig

#include "veins/base/utils/NetwToMacControlInfo.h"

using namespace veins;

namespace {

using tNetwToMacControlInfoBase = NetwToMacControlInfo;

} // namespace

cObject* const NetwToMacControlInfo::setControlInfo(cMessage* const pMsg, const LAddress::L2Type& pDestAddr)
{
    tNetwToMacControlInfoBase* const cCtrlInfo = new tNetwToMacControlInfoBase();

    cCtrlInfo->setDest(pDestAddr);
    pMsg->setControlInfo(cCtrlInfo);

    return cCtrlInfo;
}

const LAddress::L2Type& NetwToMacControlInfo::getDestFromControlInfo(const cObject* const pCtrlInfo)
{
    const tNetwToMacControlInfoBase* const cCtrlInfo = dynamic_cast<const tNetwToMacControlInfoBase*>(pCtrlInfo);

    ASSERT(cCtrlInfo);
    return cCtrlInfo->getDest();
}
