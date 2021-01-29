/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    FileLogger.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.logger;

import de.dlr.ts.commons.tools.FileTools;
import java.io.File;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.logging.Level;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
@Deprecated
public class FileLogger {
    private static FileLogger instance = new FileLogger();
    private List<String> lines = new ArrayList<String>();
    private String dir = "log";
    private String fileName;
    private static DateFormat dfmt = new SimpleDateFormat("yyyyMMdd_HH.mm.ss");


    /**
     *
     */
    public void setFileNameWithCurrentTime() {
        String format = dfmt.format(new Date(System.currentTimeMillis()));

        setFileName(format + ".txt");
    }

    /**
     *
     * @param fileName
     */
    public void setFileName(String fileName) {
        this.fileName = fileName;

        if (!new File(dir).exists()) {
            new File(dir).mkdir();
        }
    }

    /**
     *
     * @param dir
     */
    public void setDir(String dir) {
        this.dir = dir;

        if (!new File(dir).exists()) {
            new File(dir).mkdir();
        }
    }

    /**
     *
     * @return
     */
    public static FileLogger getInstance() {
        return instance;
    }

    /**
     *
     * @param line
     */
    public void addLine(String line) {
        DLRLogger.info(line);
        lines.add(DLRLogger.getLine(line, "|"));
    }

    /**
     *
     */
    public void clear() {
        lines.clear();
    }

    /**
     *
     */
    public void writeToDisc() {
        try {
            FileTools.writeSmallTextFile(dir + "/" + fileName, lines);
        } catch (IOException ex) {
            java.util.logging.Logger.getLogger(FileLogger.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}
