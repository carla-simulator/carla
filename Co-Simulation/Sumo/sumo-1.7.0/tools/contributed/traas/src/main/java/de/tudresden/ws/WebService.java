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
/// @file    WebService.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws;

import javax.xml.ws.Endpoint;

import de.tudresden.ws.conf.Config;
import de.tudresden.ws.log.Log;

/**
 *
 * @author Mario Krumnow
 *
 */

public class WebService {

    static Config conf;
    static Log logger;

    public static void main(String args[]) {

        conf = new Config();
        if (args.length == 1) {
            conf.read_config(args[0]);
        }

        logger = conf.logger;
        logger.write(conf.name + " is going to start", 1);

        Service server = new ServiceImpl(conf);
        logger.write("The webservice (Version " + server.version() + ") is available under " + conf.get_url(), 1);

        //start webservice
        Endpoint endpoint = Endpoint.publish(conf.get_url(), server);

        ShutdownHook shutdownHook = new ShutdownHook();
        Runtime.getRuntime().addShutdownHook(shutdownHook);

        while (shutdownHook.isshutdown() != true) {
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
                logger.write(e.getStackTrace());
            }
        }


        try {
            endpoint.stop();
        } catch (Exception e) {
            logger.write(e.getStackTrace());
        }

        logger.write(conf.name + " finished successfully.", 1);

    }
}
