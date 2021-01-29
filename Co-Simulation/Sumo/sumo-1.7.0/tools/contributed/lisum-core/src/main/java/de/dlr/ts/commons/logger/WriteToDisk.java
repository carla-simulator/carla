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
/// @file    WriteToDisk.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.logger;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
class WriteToDisk {

    private String logFolder = "log";
    private final String LOG_FILE_EXT = ".log";
    private final long LOG_REFRESH_RATE = 10000;

    private List<Line> logLines = Collections.synchronizedList(new ArrayList<Line>());
    private double logFolderSizeLimitMB = 0d;

    private final SimpleDateFormat sdf = new SimpleDateFormat("yyyy_MM_dd");
    //private FileWriter fr = null;

    private long todayTime;
    private String currentFileName = "";
    private Timer cleanerTimer;
    private Timer writerTimer;

    /**
     *
     * @param maxSize
     */
    public WriteToDisk(double maxSize) {
        this.logFolderSizeLimitMB = maxSize;

        if (logFolderSizeLimitMB != 0d) {
            DLRLogger.finer(this, "Setting logFolderSizeLimit to "
                            + logFolderSizeLimitMB + "MB");
        }
    }

    public void start() {
        if (logFolderSizeLimitMB != 0d) {
            cleanerTimer = new Timer(true);
            cleanerTimer.schedule(cleanerTask, 0L, 1000 * 60 * 60 * 12);
        }

        todayTime = System.currentTimeMillis();
        setCurrentFileName();

        writerTimer = new Timer(true);
        writerTimer.schedule(writerTask, LOG_REFRESH_RATE, LOG_REFRESH_RATE);

        /**
         * Shutdown hook
         */
        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                writeToDisk();  //before finishing the application
            }
        });
    }

    private TimerTask writerTask = new TimerTask() {
        @Override
        public void run() {
            String old_ = sdf.format(new Date(todayTime));
            String new_ = sdf.format(new Date(System.currentTimeMillis()));

            if (!old_.equals(new_)) {
                setCurrentFileName();
                todayTime = System.currentTimeMillis();
            }

            writeToDisk();
            cleanList();
        }
    };

    /**
     *
     * @param line
     */
    public synchronized void addLogLine(String line) {
        if (logLines.size() < 10000) { //Protection against out of momory error
            logLines.add(new Line(line));
        }
    }

    /**
     *
     */
    private void setCurrentFileName() {
        if (!new File(logFolder).exists()) {
            new File(logFolder).mkdir();
        }

        currentFileName = logFolder + File.separator
                          + getNewFileName() + LOG_FILE_EXT;

        if (!wasCleanedToday()) {
            cleanLogFolder(); //Executes this method once a day
        }
    }

    private String getNewFileName() {
        int biggest = 0;
        boolean exist = false;
        String name = sdf.format(new Date(System.currentTimeMillis()));

        for (String s : new File(logFolder).list()) {
            if (s.contains(name) && s.endsWith(LOG_FILE_EXT)) {
                exist = true;

                String tmp = s.replace(name, "").replace(".", "").replace("log", "").trim();

                try {
                    if (!tmp.isEmpty()) {
                        if (biggest < Integer.valueOf(tmp)) {
                            biggest = Integer.valueOf(tmp);
                        }
                    }
                } catch (Exception e) {
                }
            }
        }

        if (exist) {
            return name + "." + String.format("%03d", ++biggest);
        }

        return name + "." + String.format("%03d", biggest);
    }

    /**
     *
     */
    private synchronized void writeToDisk() {
        if (logLines.isEmpty()) {
            return;
        }

        FileWriter fr = null;
        try {
            fr = new FileWriter(new File(currentFileName), true);
            for (Line ll : logLines) {
                fr.append(ll.line);
                fr.append(System.getProperty("line.separator"));
                ll.processed = true;
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        } finally {
            try {
                fr.close();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
    }

    private synchronized void cleanList() {
        Iterator<Line> i = logLines.iterator();
        while (i.hasNext()) {
            if (i.next().processed) {
                i.remove();
            }
        }
    }

    /**
     *
     * @return
     */
    private boolean wasCleanedToday() {
        String name = sdf.format(new Date(System.currentTimeMillis()));

        for (String s : new File(logFolder).list()) {
            if (s.contains(name)) {
                return true;
            }
        }

        return false;
    }

    /**
     *
     * @param logFolder
     */
    public void setLogFolder(String logFolder) {
        File file = new File(logFolder);
        file = new File(file.getAbsolutePath());

        if (!file.getName().equals("log")) {
            file = new File(file.getAbsolutePath() + File.separator + "log");
        }

        File parent = new File(file.getParent());

        if (!parent.exists()) {
            DLRLogger.severe(this, "Log Folder could not be set, setting default: "
                             + new File("").getAbsolutePath() + File.separator + "log");
        }
    }

    private TimerTask cleanerTask = new TimerTask() {
        @Override
        public void run() {
            if (logFolderSizeLimitMB == 0) {
                return;
            }

            File _logFolder = new File(logFolder);

            if (!_logFolder.exists()) {
                return;
            }

            while (true) {
                if (getFolderSize(_logFolder) > logFolderSizeLimitMB) {
                    deleteOldestLogFile();
                } else {
                    break;
                }
            }
        }
    };

    /**
     * Checks if the size of the LOG folder exceeds the desired max size
     */
    private void cleanLogFolder() {
        if (logFolderSizeLimitMB == 0) {
            return;
        }

        File _logFolder = new File(logFolder);

        if (!_logFolder.exists()) {
            return;
        }

        while (true) {
            if (getFolderSize(_logFolder) > logFolderSizeLimitMB) {
                deleteOldestLogFile();
            } else {
                break;
            }
        }
    }

    /**
     *
     * @return Folder size in MB.
     */
    private double getFolderSize(File folder) {
        double size = 0d;
        for (File file : folder.listFiles()) {
            size += file.length();
        }

        return size / (1024 * 1024);
    }

    /**
     *
     */
    private void deleteOldestLogFile() {
        File _logFolder = new File(logFolder);

        List<String> asList = Arrays.asList(_logFolder.list());
        Collections.sort(asList);

        File file = new File(logFolder + "/" + asList.get(0));
        file.delete();
    }

    /**
     *
     * @param time
     */
    private static void sleepi(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException ex) {
        }
    }

    private static class Line {

        String line;
        boolean processed = false;

        public Line(String line) {
            this.line = line;
        }

        @Override
        public String toString() {
            return line;
        }
    }
}
