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
/// @file    GlobalConfig.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.tools.FileTools;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Properties;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public final class GlobalConfig {

    private static final GlobalConfig INSTANCE = new GlobalConfig();

    private static final String PERSISTENCE_FILE = "props";
    private final int sumoPort = 9100;

    //private String sumoExec = "c:/Program Files (x86)/DLR/Sumo/bin/sumo-gui.exe";
    private String sumoExec = "";
    private final String sumoExec_linux_default = "/usr/bin/sumo-gui";
    private String sumoExec_linux = sumoExec_linux_default;

    private final String defaultTextEditor = "notepad.exe";
    private String textEditor = defaultTextEditor;
    private final String defaultTextEditor_linux = "gedit";
    private String textEditor_linux = defaultTextEditor_linux;

    private final String defaultFilesExplorer = "explorer.exe";
    private String filesExplorer = defaultFilesExplorer;
    private final String defaultFilesExplorer_linux = "nautilus";
    private String filesExplorer_linux = defaultFilesExplorer_linux;

    private String workspace = "";
    private final Properties props = new Properties();

    private final String defaultLisaRestFulServerDir = "OmlFgServer";
    private String lisaRestFulServerDir = defaultLisaRestFulServerDir;

    private String lisaServerAddress = "localhost";
    private int lisaServerPort = 9091;

    private String loggingLevel = "INFO";
    private File persistenceFile;

    /**
     * Keys
     */
    private static final String LOGGING_LEVEL_KEY = "logging_level";
    private static final String RESTFUL_SERVER_DIR_KEY = "lisa_restFUL_server_dir";
    private static final String TEXT_EDITOR_KEY = "text_editor";
    private static final String TEXT_EDITOR_LINUX_KEY = "text_editor_linux";
    private static final String SUMO_EXEC_KEY = "sumo_exec";
    private static final String FILES_EXPLORER_KEY = "files_explorer";
    private static final String SUMO_EXEC_LINUX_KEY = "sumo_exec_linux";
    private static final String FILES_EXPLORER_LINUX_KEY = "files_explorer";

    /**
     *
     */
    private GlobalConfig() {
    }

    /**
     *
     */
    private void update() {
        setPersistenceFile();
        readProps();
        updateLisaServerParameters();
        saveProps();
    }

    /**
     *
     */
    private void updateLisaServerParameters() {
        String fileName = lisaRestFulServerDir + File.separator + "OmlFgServer.ini";
        try {
            List<String> lines = FileTools.readSmallTextFile(fileName);
            String _line = "";

            for (String line : lines) {
                if (line.contains("pdBase")) {
                    _line = line;
                }
            }

            _line = _line.replaceAll("pdBase=http://", "");
            _line = _line.replaceAll("/", "");

            String[] split = _line.split(":");
            lisaServerAddress = split[0];
            lisaServerPort = Integer.valueOf(split[1]);
        } catch (IOException ex) {
            DLRLogger.severe(this, lisaRestFulServerDir + File.separator + "OmlFgServer.ini" + " file not found");
        }
    }

    /**
     *
     */
    private void setPersistenceFile() {
        String tmpdir = workspace + File.separator + ".metadata" + File.separator;
        persistenceFile = new File(tmpdir + PERSISTENCE_FILE);
    }

    /**
     *
     */
    private void readProps() {
        try {
            if (persistenceFile.exists()) {
                props.load(new FileInputStream(persistenceFile));

                String logLevel = props.getProperty(LOGGING_LEVEL_KEY);
                if (logLevel != null) {
                    this.loggingLevel = logLevel;
                }

                String _lisaDir = props.getProperty(RESTFUL_SERVER_DIR_KEY);
                if (_lisaDir != null) {
                    this.lisaRestFulServerDir = _lisaDir;
                }

                String _textEditor = props.getProperty(TEXT_EDITOR_KEY);
                if (_textEditor != null) {
                    this.textEditor = _textEditor;
                }

                String _sumoExec = props.getProperty(SUMO_EXEC_KEY);
                if (_sumoExec != null) {
                    this.sumoExec = _sumoExec;
                }

                String files_ex = props.getProperty(FILES_EXPLORER_KEY);
                if (files_ex != null) {
                    this.filesExplorer = files_ex;
                }

                String __textEditor = props.getProperty(TEXT_EDITOR_LINUX_KEY);
                if (__textEditor != null) {
                    this.textEditor_linux = __textEditor;
                }

                String __sumoExec = props.getProperty(SUMO_EXEC_LINUX_KEY);
                if (__sumoExec != null) {
                    this.sumoExec_linux = __sumoExec;
                }

                String files__ex = props.getProperty(FILES_EXPLORER_LINUX_KEY);
                if (files__ex != null) {
                    this.filesExplorer_linux = files__ex;
                }

            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    /**
     *
     * @return
     */
    public String getLisaRESTfulServerPath() {
        return lisaRestFulServerDir;
    }

    /**
     *
     */
    public void saveProps() {
        props.put(TEXT_EDITOR_KEY, textEditor);
        props.put(SUMO_EXEC_KEY, sumoExec);
        props.put(FILES_EXPLORER_KEY, filesExplorer);
        props.put(LOGGING_LEVEL_KEY, loggingLevel);
        props.put(RESTFUL_SERVER_DIR_KEY, lisaRestFulServerDir);
        props.put(TEXT_EDITOR_LINUX_KEY, textEditor_linux);
        props.put(SUMO_EXEC_LINUX_KEY, sumoExec_linux);
        props.put(FILES_EXPLORER_LINUX_KEY, filesExplorer_linux);

        DLRLogger.finest(this, "Saving preferences to " + persistenceFile);
        DLRLogger.finest(this, props.toString());

        try {
            props.store(new FileOutputStream(persistenceFile), "");
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    /**
     *
     * @return
     */
    public static GlobalConfig getInstance() {
        return INSTANCE;
    }

    /**
     *
     * @return
     */
    public String getDefaultTextEditor() {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            return defaultTextEditor;
        }

        return defaultTextEditor_linux;
    }

    /**
     *
     * @return
     */
    public String getTextEditor() {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            return textEditor;
        }

        return textEditor_linux;
    }

    /**
     *
     * @param textEditor
     */
    public void setTextEditor(String textEditor) {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            this.textEditor = textEditor;
        } else {
            this.textEditor_linux = textEditor;
        }
    }

    /**
     *
     * @param sumoExec
     */
    public void setSumoExec(String sumoExec) {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            this.sumoExec = sumoExec;
        } else {
            this.sumoExec_linux = sumoExec;
        }
    }

    /**
     *
     * @return
     */
    public String getWorkspace() {
        return workspace;
    }

    /**
     *
     * @param workspace
     */
    public void setWorkspace(String workspace) {
        if (!this.workspace.equals(workspace)) {
            this.workspace = workspace;
            update();
        }
    }

    /**
     *
     * @return
     */
    public String getSumoExec() {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            return sumoExec;
        }

        return sumoExec_linux;
    }

    /**
     *
     * @return
     */
    public int getSumoPort() {
        return sumoPort;
    }

    public int getLisaServerPort() {
        return lisaServerPort;
    }

    public String getLisaServerAddress() {
        return lisaServerAddress;
    }

    public String getLoggingLevel() {
        return loggingLevel;
    }

    public void setLoggingLevel(String loggingLevel) {
        this.loggingLevel = loggingLevel;
    }

    public String getDefaultLisaRestFulServerDir() {
        return defaultLisaRestFulServerDir;
    }

    public void setLisaRestFulServerDir(String lisaRestFulServerDir) {
        if (!lisaRestFulServerDir.equals(this.lisaRestFulServerDir)) {
            this.lisaRestFulServerDir = lisaRestFulServerDir;
            updateLisaServerParameters();
        }
    }

    public String getFilesExplorer() {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            return filesExplorer;
        }

        return filesExplorer_linux;
    }

    public String getDefaultFilesExplorer() {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            return defaultFilesExplorer;
        }

        return defaultFilesExplorer_linux;
    }

    public void setFilesExplorer(String filesExplorer) {
        if (System.getProperty("os.name").toUpperCase().contains("WIN")) {
            this.filesExplorer = filesExplorer;
        } else {
            this.filesExplorer_linux = filesExplorer;
        }
    }

}
