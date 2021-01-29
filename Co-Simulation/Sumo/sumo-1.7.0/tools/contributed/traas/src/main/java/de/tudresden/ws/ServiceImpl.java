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
/// @file    ServiceImpl.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws;

import javax.jws.WebMethod;
import javax.jws.WebParam;
import javax.jws.WebResult;
import javax.jws.WebService;
import javax.xml.ws.BindingType;
import javax.xml.ws.soap.SOAPBinding;

import de.tudresden.sumo.util.ConvertHelper;
import de.tudresden.sumo.util.Sumo;
import de.tudresden.ws.conf.Config;
import de.tudresden.ws.log.Log;

@WebService(serviceName = "TraasWS")

/**
 *
 * @author Mario Krumnow
 *
 */

public class ServiceImpl extends Traci implements Service {

    Log logger;
    Config conf;
    Sumo sumo;
    ConvertHelper helper;

    public ServiceImpl(Config conf) {
        this.conf = conf;
        this.logger = conf.logger;
        this.conf.refresh_actiontime();
        this.helper = new ConvertHelper(this.logger);
    }

    @WebMethod(action = "start service")
    public String start(@WebParam(name = "user") String user) {

        String output = "failed";

        if (!this.conf.running) {
            conf.running = true;
            logger.write("Benutzer " + user + " startet den " + conf.name + " Service", 1);

            sumo = new Sumo(this.conf);
            sumo.start_ws();

            //init super class
            super.init(sumo, logger, helper);

            this.conf.refresh_actiontime();
            output = "success";
        }


        return output;

    }

    @WebMethod(action = "stop service")
    public String stop(@WebParam(name = "user") String user) {

        sumo.stop_instance();
        conf.running = false;
        return "success";

    }

    @WebMethod(action = "status")
    public String get_Status(@WebParam(name = "user") String user) {
        this.conf.refresh_actiontime();
        return "Running: " + conf.running;
    }


    @WebMethod(action = "last action time")
    public String LastActionTime() {
        return conf.get_actiontime();
    }

    @WebMethod(action = "version info")
    @WebResult(name = "output")
    public String version() {
        return conf.version;
    }

    @WebMethod(action = "activate text output. only for debugging purposes.")
    public String TXT_output(@WebParam(name = "active") boolean input) {
        logger.txt_output(input);
        return "success";
    }


    @WebMethod(action = "add Options to sumo binary")
    public void addOption(String name, String value) {

        if (!conf.running) {
            this.conf.sumo_output.put(name, value);
        }


    }

    @WebMethod(action = "do timestep")
    public void doTimestep() {

        if (conf.running) {
            this.sumo.do_timestep();
        }

    }

    @SuppressWarnings("static-access")
    @WebMethod(action = "set the configuration file")
    public void setConfig(@WebParam(name = "filename") String filename) {

        if (!this.conf.running) {
            Config.config_file = filename;
        }

    }

    @SuppressWarnings("static-access")
    @WebMethod(action = "set the sumo binary")
    public void setSumoBinary(@WebParam(name = "binary") String filename) {

        if (!this.conf.running) {
            Config.sumo_bin = filename;
        }

    }


}
