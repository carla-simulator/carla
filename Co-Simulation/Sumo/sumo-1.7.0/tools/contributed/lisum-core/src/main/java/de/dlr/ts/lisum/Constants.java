/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    Constants.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    Constants.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class Constants {
    /**
     *
     */
    public static final String DEFAULT_LOG_LEVEL = "INFO";

    /**
     * Properties keys
     */
    public static final String LISA_RESTFUL_SERVER_DIR = "lisa.RESTfulServerDir";
    public static final String LISA_RESTFUL_SERVER_ADDRESS = "lisa.serverAddress";
    public static final String LISA_RESTFUL_SERVER_PORT = "lisa.serverPort";
    //public static final String LISA_DATA_DIR = "lisa.dataDir";

    /**
     *
     */
    public static final String GLOBAL_CONFIG_FILE_NAME = "config.xml";

    /**
     * Global config file keys
     */
    public static final String LISA_RESTFUL_SERVER_DIR_KEY = "lisa.RESTfulServer.directory";
    public static final String LISA_RESTFUL_SERVER_ADDRESSS_KEY = "lisa.RESTfulServer.address";
    public static final String LISA_RESTFUL_SERVER_PORT_KEY = "lisa.RESTfulServer.port";
    public static final String SUMO_EXEC_KEY = "sumo.exec";

    /**
     * LiSuM config file keys
     */
    public static final String LISA_FILES_DIRECTORY_KEY = "input.lisa";
    public static final String CONTROLUNITS_ITEM_KEY = "controlUnits.item";
    public static final String DETECTORS_ITEM_KEY = "detectors.item";

    /**
     *
     */
    public static final int LISA_RESTFUL_SERVER_PORT_DEFAULT = 9091;

    public static final List<String> ebeneItems = Arrays.asList(
                "Serviceeingriff",
                "Bediener, lokal",
                "Programm fix, lokal",
                "Handrastbetrieb, lokal",
                "Schaltuhr, lokal",
                "Zentrale",
                "Zentrale 2",
                "Sondereingriff-Zentrale",
                "Sondereingriff",
                "Logik",
                "Fehler Gerät",
                "Signalsicherung",
                "System"
            );
}
