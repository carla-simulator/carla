/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2016-2017 Dresden University of Technology
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
/// @file    ContextSubscription.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.subscription;

import java.util.LinkedList;

import de.tudresden.sumo.config.Constants;
import it.polito.appeal.traci.protocol.Command;

public class ContextSubscription implements Subscription  {

    public SubscriptionContext context;
    public double start;
    public double stop;
    public String objectID;
    public SumoDomain contextDomain;
    public double contextRange;
    public LinkedList<Integer> commands;

    public ContextSubscription() {

        //demo
        this.context = SubscriptionContext.lane;
        this.start = 0;
        this.stop = 100000 * 60;
        this.objectID = "gneE0_0";
        this.contextDomain = SumoDomain.vehicles;
        this.contextRange = 100;

        this.commands = new LinkedList<Integer>();
        this.commands.add(Constants.VAR_POSITION);
        this.commands.add(Constants.VAR_SPEED);

    }

    public ContextSubscription(SubscriptionContext context, double start, double stop, String objectID, SumoDomain contextDomain, double contextRange) {

        this.context = context;
        this.start = start;
        this.stop = stop;
        this.objectID = objectID;
        this.contextDomain = contextDomain;
        this.contextRange = contextRange;
        this.commands = new LinkedList<Integer>();
    }

    public void addCommand(int cmd) {
        this.commands.add(cmd);
    }

    public Command getCommand() {

        Command cmd = new Command(this.context.id);
        cmd.content().writeDouble(this.start);
        cmd.content().writeDouble(this.stop);
        cmd.content().writeStringASCII(this.objectID);
        cmd.content().writeUnsignedByte(this.contextDomain.id);
        cmd.content().writeDouble(this.contextRange);
        cmd.content().writeUnsignedByte(this.commands.size());
        for (Integer ix : this.commands) {
            cmd.content().writeUnsignedByte(ix);
        }

        return cmd;
    }

}
