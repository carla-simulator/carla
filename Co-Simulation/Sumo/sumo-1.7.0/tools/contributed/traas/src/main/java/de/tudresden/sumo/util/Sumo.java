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
/// @file    Sumo.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.util;

import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

import it.polito.appeal.traci.SumoTraciConnection;
import de.tudresden.sumo.util.SumoCommand;
import de.tudresden.ws.conf.Config;
import de.tudresden.ws.log.Log;

/**
 *
 * @author Mario Krumnow
 *
 */

public class Sumo {

    Log logger;
    Config conf;

    public SumoTraciConnection conn;



    boolean running = false;

    //default constructor
    public Sumo() {}

    public Sumo(Config conf) {
        this.conf = conf;
    }

    public void start(String sumo_bin, String configFile) {

        //start SUMO
        conn = new SumoTraciConnection(sumo_bin, configFile);
        try {
            conn.runServer();
            this.running = true;

        } catch (Exception ex) {
            ex.printStackTrace();
            this.running = false;
        }
    }

    public void start(String sumo_bin, String net_file, String route_file) {

        //start SUMO
        conn = new SumoTraciConnection(
            sumo_bin, //binary
            net_file,   // net file
            route_file // route file
        );

        try {
            conn.runServer();
            this.running = true;

        } catch (Exception ex) {
            ex.printStackTrace();
            this.running = false;
        }
    }

    public boolean set_cmd(Object in) {

        boolean output = false;
        if (this.running) {
            try {
                this.conn.do_job_set((SumoCommand) in);
                output = true;
            } catch (Exception ex) {
                this.logger.write(ex.getStackTrace());
            }
        }

        return output;

    }

    public Object get_cmd(SumoCommand cmd) {

        Object obj = -1;
        if (this.running) {
            try {
                obj = conn.do_job_get(cmd);
            } catch (Exception ex) {
                this.logger.write(ex.getStackTrace());
            }
        }
        return obj;
    }

    public void do_timestep() {

        try {
            conn.do_timestep();
        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

    }

    @SuppressWarnings("static-access")
    public void start_ws() {

        //start SUMO
        conn = new SumoTraciConnection(Config.sumo_bin, Config.config_file);

        //Add Options
        this.add_options();

        try {
            conn.runServer();
            this.running = true;
        } catch (Exception ex) {
            logger.write(ex.getStackTrace());
            this.running = false;
        }

    }

    private void add_options() {

        Set<Entry<String, String>> set = this.conf.sumo_output.entrySet();
        Iterator<Entry<String, String>> it = set.iterator();
        while (it.hasNext()) {
            Entry<String, String> option = it.next();
            conn.addOption(option.getKey(), option.getValue());
        }

    }

    public void stop_instance() {

        try {

            conn.close();
            this.running = false;

        } catch (Exception ex) {
            logger.write(ex.getStackTrace());
        }

    }

}
