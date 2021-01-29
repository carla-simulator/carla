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
/// @file    ShutdownHook.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws;

/**
 *
 * @author Mario Krumnow
 *
 */

public class ShutdownHook  extends Thread {

    private boolean shutdown;

    /**
     * Instantiates a new shutdown hook.
     */
    public ShutdownHook() {
        setshutdown(false);
    }

    public void run() {
        System.out.println("Shutdown in progress...");

        setshutdown(true);

        try {
            Thread.sleep(3000);
        } catch (Exception e) {}

        System.out.println("Shutdown finished");

    }


    public void setshutdown(boolean shutdown) {
        this.shutdown = shutdown;
    }

    public boolean isshutdown() {
        return shutdown;
    }


}
