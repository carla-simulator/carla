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
/// @file    Config.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws.conf;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import de.tudresden.ws.log.Log;
import de.tudresden.ws.log.Log_txt;

/**
 *
 * @author Mario Krumnow
 *
 */

public class Config {

    public boolean running = false;

    public SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    public static String host = "127.0.0.1";
    public static String port = "4223";
    public static String url = "SUMO";
    public String name = "Sumo Webservice";
    public static String sumo_bin = "f:/Programme/sumo-svn/bin/sumo-gui64.exe";
    public static String config_file = "simulation/lsa/config.sumo.cfg";

    public HashMap<String, String> sumo_output = new HashMap<String, String>();
    public Date lastactiontime;

    public String version = "1.1";
    public Log logger;

    public Config() {

        logger = new Log_txt();
        refresh_actiontime();
    }

    public boolean read_config(String filename) {

        boolean valid = false;

        try {

            File file = new File(filename);
            if (file.exists()) {

                try {
                    SAXParserFactory factory = SAXParserFactory.newInstance();
                    SAXParser saxParser = factory.newSAXParser();
                    saxParser.parse(file, get_handler());
                } catch (Exception ex) {
                    ex.printStackTrace();
                }

            } else {
                System.out.println("The file " + file.getAbsolutePath() + " does not exist.");
            }


        } catch (Exception ex) {
            ex.printStackTrace();
        }

        return valid;
    }

    public String get_url() {
        String out = "http://" + host + ":" + port + "/" + url;
        return out;
    }

    public void refresh_actiontime() {
        this.lastactiontime = new Date();
    }

    public String get_actiontime() {
        return df.format(this.lastactiontime);
    }

    private static DefaultHandler get_handler() {

        DefaultHandler handler = new DefaultHandler() {

            public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {

                if (qName.equals("host")) {
                    host = attributes.getValue("value");
                }
                if (qName.equals("port")) {
                    port = attributes.getValue("value");
                }
                if (qName.equals("name")) {
                    url = attributes.getValue("value");
                }
                if (qName.equals("sumo_bin")) {
                    sumo_bin = attributes.getValue("value");
                }
                if (qName.equals("config_file")) {
                    config_file = attributes.getValue("value");
                }

            }

            public void endElement(String uri, String localName, String qName) throws SAXException {}

            public void characters(char ch[], int start, int length) throws SAXException {}


        };


        return handler;

    }


}
