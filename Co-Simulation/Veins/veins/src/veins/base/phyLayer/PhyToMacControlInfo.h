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

#pragma once

#include "veins/veins.h"

#include "veins/base/phyLayer/Decider.h"

namespace veins {

/**
 * @brief Controlinfo for packets which are send from Physical
 * layer to the MAC layer.
 *
 * The ControlInfo contains the the DeciderResult of the Decider.
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class VEINS_API PhyToMacControlInfo : public cObject {
protected:
    /** The result of the decider evaluation.*/
    DeciderResult* result;

public:
    /**
     * @brief Initializes the PhyToMacControlInfo with the passed DeciderResult.
     *
     * NOTE: PhyToMacControlInfo takes ownership of the passed DeciderResult!
     */
    PhyToMacControlInfo(DeciderResult* result)
        : result(result)
    {
    }

    /**
     * @brief Clean up the DeciderResult.
     */
    ~PhyToMacControlInfo() override
    {
        if (result) delete result;
    }

    /**
     * @brief Returns the result of the evaluation of the Decider.
     */
    DeciderResult* getDeciderResult() const
    {
        return result;
    }

    /**
     * @brief Attaches a "control info" structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the decider result.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg                The message where the "control info" shall be attached.
     * @param pDeciderResult    The decider results.
     */
    static cObject* const setControlInfo(cMessage* const pMsg, DeciderResult* const pDeciderResult)
    {
        PhyToMacControlInfo* const cCtrlInfo = new PhyToMacControlInfo(pDeciderResult);
        pMsg->setControlInfo(cCtrlInfo);

        return cCtrlInfo;
    }
    /**
     * @brief extracts the decider result from message "control info".
     */
    static DeciderResult* const getDeciderResult(cMessage* const pMsg)
    {
        return getDeciderResultFromControlInfo(pMsg->getControlInfo());
    }
    /**
     * @brief extracts the decider result from message "control info".
     */
    static DeciderResult* const getDeciderResultFromControlInfo(cObject* const pCtrlInfo)
    {
        PhyToMacControlInfo* const cCtrlInfo = dynamic_cast<PhyToMacControlInfo* const>(pCtrlInfo);

        if (cCtrlInfo) return cCtrlInfo->getDeciderResult();
        return nullptr;
    }
};

} // namespace veins
