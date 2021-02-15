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
/// @file    Main.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package de.dlr.ts.lisum;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.logger.LogLevel;
import de.dlr.ts.commons.tools.FileTools;
import de.dlr.ts.lisum.interfaces.ControlUnitInterface;
import de.dlr.ts.lisum.interfaces.SignalProgramInterface;
import de.dlr.ts.lisum.simulation.LisumSimulation;
import de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException;
import de.dlr.ts.utils.xmladmin2.XMLAdmin2;
import de.dlr.ts.utils.xmladmin2.XMLNode;
import de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.xml.sax.SAXException;

/**
 *
 * @author bott_ma
 */
public class Main {
    private String sumoExec = "sumo";
    private String sumoConfig = null;
    private final int sumoPort = 9100;
    private String lisaRestFulServerDir = "localhost";
    private final int lisaPort = 9091;
    private String lisumFile = "";
    private String loggingLevel = "INFO";
    private LisumSimulation ls;
    private boolean pause = false;

    /**
     * Starts lisum-core
     *
     * @param args
     * @throws Exception
     */
    public static void main(String[] args) throws Exception {
        SplashScreen sc = new SplashScreen("LiSuM 1.0.1");
        sc.setYear(2018);

        sc.addRuntimeOption("-log            ", "logging level", "INFO");
        sc.addRuntimeOption("-f or -file     ", "lisum_xml Project file", "");
        sc.addRuntimeOption("-c or -conf     ", "config file", "");
        sc.addRuntimeOption("-s or -sumoexec ", "Sumo executable", "");
        sc.addRuntimeOption("-S or -sumocfg  ", "Sumo configuration", "");
        sc.addRuntimeOption("-p              ", "Pause before starting", "");
        //sc.addRuntimeOption("-l or -lisa     ", "lisa RESTful server address", "localhost");
        sc.showSplashScreen();

        new Main().start(args);
    }

    private void readConfFile(String file) {

        DLRLogger.info("Reading config file: " + file.trim());

        try {
            List<String> f = FileTools.readSmallTextFile(file);

            StringBuilder sb = new StringBuilder();

            for (String string : f) {
                if (!string.trim().startsWith("#")) {
                    sb.append(string + " ");
                }
            }

            readConf(sb.toString().split(" "));

        } catch (IOException ex) {
            Logger.getLogger(Main.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    private void readConf(String[] args) {
        args = cleanArray(args);

        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-conf") || args[i].equals("-c")) {
                readConfFile(args[i + 1]);
            }

            if (args[i].equals("-sumoexec") || args[i].equals("-s")) {
                sumoExec = args[i + 1];
                DLRLogger.info("Setting sumoExec: " + sumoExec);

                if (!new File(sumoExec).exists()) {
                    DLRLogger.severe(String.format("Error: Couldn't find Sumo exec file (%s). Quitting.", sumoExec));
                    System.exit(0);
                }
            }

            if (args[i].equals("-sumocfg") || args[i].equals("-S")) {
                sumoConfig = args[i + 1];
                DLRLogger.info("Setting sumoConfig: " + sumoConfig);

                if (!new File(sumoConfig).exists()) {
                    DLRLogger.severe(String.format("Error: Couldn't find Sumo coniguration file (%s). Quitting.", sumoConfig));
                    System.exit(0);
                }
            }

            if (args[i].equals("-p")) {
                pause = true;
            }

            if (args[i].equals("-lisa") || args[i].equals("-l")) {
                lisaRestFulServerDir = args[i + 1];
                DLRLogger.info("Setting Lisa Server: " + lisaRestFulServerDir);
            }

            if (args[i].equals("-log")) {
                loggingLevel = args[i + 1];
                DLRLogger.info("Setting logging level: " + loggingLevel.toUpperCase());
            }

            if (args[i].equals("-file") || args[i].equals("-f")) {
                lisumFile = args[i + 1];
                DLRLogger.info("Project file: " + lisumFile);

                if (!new File(lisumFile).exists()) {
                    DLRLogger.severe(String.format("Error: Couldn't find projec file (%s). Quitting.", lisumFile));
                    System.exit(0);
                }
            }
        }
    }

    /**
     *
     * @param args
     * @throws Exception
     */
    public void start(String[] args) throws Exception {
        readConf(args);

        DLRLogger.setLevel(LogLevel.valueOf(loggingLevel.toUpperCase()));

        if (lisumFile.isEmpty()) {
            System.err.println("Error: No Lisum file received. Use -f <path_to_file/lisum.xml>. Quitting.");
            return;
        }

        ls = new LisumSimulation(sumoExec, sumoPort, lisaRestFulServerDir, lisaPort);
        ls.setSumoConfig(sumoConfig);
        ls.load(new File(lisumFile));

        LisumSimulation.InitBeforePlayResponse initBeforePlay = ls.initBeforePlay();

        if (initBeforePlay == LisumSimulation.InitBeforePlayResponse.LisaRESTfulServerNotFound) {
            System.err.println(String.format("Error: No Lisa RESTfull server found in localhost:%d. Quitting.", this.lisaPort));
        } else {

            DLRLogger.info("");

            DLRLogger.info("******  Ebene items  ******");
            for (int i = 0; i < Constants.ebeneItems.size(); i++) {
                DLRLogger.info(String.format("%1$4s", i) + " " + Constants.ebeneItems.get(i));
            }

            DLRLogger.info("");
            DLRLogger.info("******  Available Control Units  ******");

            for (ControlUnitInterface cu : ls.getCityInterface().getControlUnits()) {
                DLRLogger.info("> " + cu.getFullName());

                for (SignalProgramInterface sp : cu.getSignalPrograms()) {
                    DLRLogger.info("      o " + sp.getName());
                }
            }
            //DLRLogger.info("");

            loadDefaults();

            if (pause) {
                DLRLogger.info("Project loaded successfully. Start (Y/N)? [Y]");
                int read = System.in.read();

                if (read == 121 || read == 89 || read == 13) {
                    start();
                } else {
                    DLRLogger.info("Good bye!");
                }
            } else {
                start();
            }
        }
    }

    private void start() {
        new Thread(ls.getRunnable()).start();
    }

    private ControlUnitInterface getControlUnit(String name) {
        for (ControlUnitInterface cu : ls.getCityInterface().getControlUnits())
            if (cu.getFullName().contains(name.trim())) {
                return cu;
            }

        return null;
    }

    private void loadDefaults() {
        try {
            XMLAdmin2 x = new XMLAdmin2().load(this.lisumFile);

            ControlUnitInterface controlUnit = null;
            SignalProgramInterface signalProgram = null;
            boolean coordinated = false;
            int ebene = 0;
            boolean va = false;
            boolean iv = false;
            boolean ov = false;

            //DLRLogger.info("Reading default states in lisum.xml");
            /*
            if(!x.hasNode("defaultStates")) {
                DLRLogger.info("******  No default states definition found in lisum.xml ******");
                return;
            }*/

            for (ControlUnitInterface cu : ls.getCityInterface().getControlUnits()) {
                cu.setEnabled(false);
            }

            int count = x.getNode("controlUnits").getNodesCount("controlUnit");
            for (int i = 0; i < count; i++) {
                XMLNode node = x.getNode("controlUnits").getNode("controlUnit", i);
                String cuName = node.getAttributes().get("lisa").getValue();

                if (!node.hasNode("state")) {
                    continue;
                }

                node = x.getNode("controlUnits").getNode("controlUnit", i).getNode("state");

                boolean enabled = node.getAttributes().get("enabled").getValue(0) == 1;

                controlUnit = getControlUnit(cuName);

                if (controlUnit != null) {
                    if (!enabled) {
                        continue;
                    }

                    String programName = node.getAttributes().get("program").getValue();

                    int signalIndex = 0;
                    for (SignalProgramInterface sp : controlUnit.getSignalPrograms()) {
                        if (sp.getName().contains(programName)) {
                            signalProgram = sp;
                            break;
                        }
                        signalIndex++;
                    }

                    if (signalProgram == null) {
                        DLRLogger.severe("No signal program containing " + programName + " could be found.");
                        return;
                    }

                    controlUnit.setEnabled(true);
                    coordinated = node.getAttributes().get("coordinated").getValue(0) == 1;
                    ebene = node.getAttributes().get("ebene").getValue(0);
                    va = node.getAttributes().get("va").getValue(0) == 1;
                    iv = node.getAttributes().get("iv").getValue(0) == 1;
                    ov = node.getAttributes().get("ov").getValue(0) == 1;

                    controlUnit.setEnabled(true);
                    controlUnit.setEbene(ebene);
                    controlUnit.setCoordinated(coordinated ? 1 : 0);
                    controlUnit.setCurrentSignalProgram(signalIndex);
                    controlUnit.setVA(va);
                    controlUnit.setIV(iv);
                    controlUnit.setOV(ov);
                } else {
                    DLRLogger.severe("Error: Couldn't find control unit " + cuName);
                }
            }

            DLRLogger.info("");
            DLRLogger.info("******  Default states  ******");

            for (int j = 0; j < ls.getCityInterface().getControlUnits().length; j++) {
                controlUnit = ls.getCityInterface().getControlUnits()[j];
                DLRLogger.info("> Control unit: " + controlUnit.getFullName() + " (enabled=" + controlUnit.isEnabled() + ")");

                if (!controlUnit.isEnabled()) {
                    continue;
                }

                DLRLogger.info("     o Program: " + controlUnit.getCurrentSignalProgram().getName());
                DLRLogger.info("     o Coordinated=" + (controlUnit.getCoordinated() == 1));
                DLRLogger.info("     o Ebene=" + Constants.ebeneItems.get(ebene));
                DLRLogger.info("     o va=" + controlUnit.isVA() + ", iv=" +
                               controlUnit.isIV() + ", öv=" + controlUnit.isOV());
            }

        } catch (SAXException | XMLNodeNotFoundException | IOException | MalformedKeyOrNameException ex) {
            Logger.getLogger(Main.class.getName()).log(Level.SEVERE, null, ex);
            System.exit(0);
        }
    }


    private String[] cleanArray(String[] args) {
        List<String> tmp = new ArrayList<>();

        for (String arg : args)
            if (!arg.trim().isEmpty()) {
                tmp.add(arg.trim());
            }

        return tmp.toArray(new String[tmp.size()]);
    }
}
