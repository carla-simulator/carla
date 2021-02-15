//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Daniel Willkomm
// part of:     framework implementation developed by tkn
// description: application layer: general class for the application layer
//              subclass to create your own application layer

#pragma once

#include "veins/veins.h"

#include "veins/base/modules/BaseLayer.h"
#include "veins/base/utils/SimpleAddress.h"

namespace veins {

/**
 * @brief Base class for the application layer
 *
 * This is the generic class for all application layer modules. If you
 * want to implement your own application layer you have to subclass your
 * module from this class.
 *
 * @ingroup applLayer
 * @ingroup baseModules
 *
 * @author Daniel Willkomm
 **/
class VEINS_API BaseApplLayer : public BaseLayer {
public:
    /** @brief The message kinds this layer uses.*/
    enum BaseApplMessageKinds {
        /** Stores the id on which classes extending BaseAppl should
         * continue their own message kinds.*/
        LAST_BASE_APPL_MESSAGE_KIND = 25000,
    };
    /** @brief The control message kinds this layer uses.*/
    enum BaseApplControlKinds {
        /** Stores the id on which classes extending BaseAppl should
         * continue their own control kinds.*/
        LAST_BASE_APPL_CONTROL_KIND = 25500,
    };

protected:
    /**
     * @brief Length of the ApplPkt header
     **/
    int headerLength;

public:
    // Module_Class_Members(BaseApplLayer, BaseLayer, 0);
    BaseApplLayer()
        : BaseLayer()
    {
    }
    BaseApplLayer(unsigned stacksize)
        : BaseLayer(stacksize)
    {
    }

    /** @brief Initialization of the module and some variables*/
    void initialize(int) override;

protected:
    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     *
     * These are the functions provided to add own functionality to your
     * modules. These functions are called whenever a self message or a
     * data message from the upper or lower layer arrives respectively.
     *
     **/
    /*@{*/

    /**
     * @brief Handle self messages such as timer...
     *
     * Define this function if you want to process timer or other kinds
     * of self messages
     **/
    void handleSelfMsg(cMessage* msg) override
    {
        EV << "BaseApplLayer: handleSelfMsg not redefined; delete msg\n";
        delete msg;
    };

    /**
     * @brief Handle messages from lower layer
     *
     * Redefine this function if you want to process messages from lower
     * layers.
     *
     * The basic application layer just silently deletes all messages it
     * receives.
     **/
    void handleLowerMsg(cMessage* msg) override
    {
        EV << "BaseApplLayer: handleLowerMsg not redefined; delete msg\n";
        delete msg;
    };

    /**
     * @brief Handle control messages from lower layer
     *
     * The basic application layer just silently deletes all messages it
     * receives.
     **/
    void handleLowerControl(cMessage* msg) override
    {
        EV << "BaseApplLayer: handleLowerControl not redefined; delete msg\n";
        delete msg;
    };

    /** @brief Handle messages from upper layer
     *
     * This function is pure virtual here, because there is no
     * reasonable guess what to do with it by default.
     */
    void handleUpperMsg(cMessage* msg) override
    {
        ASSERT(false);
        throw cRuntimeError("Application has no upper layers!");
        delete msg;
    }

    /** @brief Handle control messages from upper layer */
    void handleUpperControl(cMessage* msg) override
    {
        ASSERT(false);
        throw cRuntimeError("Application has no upper layers!");
        delete msg;
    }

    /*@}*/

    /** @brief Sends a message delayed to the lower layer*/
    void sendDelayedDown(cMessage*, simtime_t_cref);

    /**
     * @brief Return my application layer address
     *
     * We use the node module index as application address
     **/
    virtual const LAddress::L3Type myApplAddr() const
    {
        return LAddress::L3Type(getParentModule()->getIndex());
    };
};

} // namespace veins
